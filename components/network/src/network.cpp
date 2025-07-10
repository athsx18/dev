#include "network.h"
#include "config.h"
#include "globals.h"
#include "secrets.h"
#include "sensor.h"
#include "utils.h"
#include "alert.h"
#include "config_manager.h"
#include <ArduinoJson.h>

unsigned long lastReconnectTime = 0;
bool justReconnected = false;


class MyBLECallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        Serial.println("BLE client connected");
        lcd.clear(); lcd.setCursor(0, 0); lcd.print("BLE Connected");
    }

    void onDisconnect(NimBLEServer* pServer) {
        Serial.println("BLE client disconnected");
        lcd.clear(); lcd.setCursor(0, 0); lcd.print("BLE Disconnected");
    }
};

void setupBLE() {
    NimBLEDevice::init("ESP32_GasMonitor");
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyBLECallbacks());

    NimBLEService *pService = pServer->createService(SERVICE_UUID);
    pGasCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID,
                        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    pGasCharacteristic->setValue("0.0");
    pService->start();
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();

    Serial.println("BLE service started");
}

void setupAWSMQTTClient() {
    net.setCACert(AWS_CERT_CA);
    net.setCertificate(AWS_CERT_CRT);
    net.setPrivateKey(AWS_CERT_PRIVATE);

    client.setServer(AWS_IOT_ENDPOINT, 8883);
    client.setKeepAlive(60);          // ✅ Improve reliability
    client.setBufferSize(1024);       // ✅ Prevent truncation of large payloads
}

bool reconnectAWS() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("🔁 WiFi not connected. Reconnecting...");
        connectToWiFi();
    }

    if (client.connected()) return true;

    Serial.print("🔌 Connecting to AWS IoT MQTT... ");
    int attempts = 0;
    unsigned long start = millis();

    while (!client.connected() && attempts++ < 5) {
        if (client.connect(THINGNAME)) {
            unsigned long elapsed = millis() - start;
            Serial.printf("✅ Connected! (%.2f sec)\n", elapsed / 1000.0);
            justReconnected = true;
            lastReconnectTime = millis();
            return true;
        }
        delay(1000);
        Serial.print(".");
    }

    Serial.println("❌ MQTT connection failed.");
    return false;
}



void connectToWiFi() {
  Config cfg = getConfig();  // 🔁 Load saved config from SPIFFS

  // 📌 Set WiFi mode and disconnect any previous connection
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(1000);

  // 🔌 Try saved credentials first
  Serial.printf("\n🔁 Trying saved WiFi: %s...\n", cfg.wifiSSID);
  WiFi.begin(cfg.wifiSSID, cfg.wifiPassword);

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Connected using saved credentials!");
    Serial.print("📍 IP: ");
    Serial.println(WiFi.localIP());
    return;
  }

  Serial.println("\n⚠️ Failed to connect using saved credentials.");

  // 📡 Fallback: Scan for available networks
  while (true) {
    Serial.println("\nScanning for WiFi networks...");
    int numNetworks = WiFi.scanNetworks();

    if (numNetworks == 0) {
      Serial.println("No networks found. Retrying...");
      delay(2000);
      continue;
    }

    Serial.println("Networks found:");
    for (int i = 0; i < numNetworks; ++i) {
      Serial.printf("[%d] %s (RSSI: %d)%s\n",
                    i + 1,
                    WiFi.SSID(i).c_str(),
                    WiFi.RSSI(i),
                    (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " [Open]" : "");
    }

    Serial.println("\nEnter the number of the SSID you want to connect to (0 to rescan):");
    while (!Serial.available()) delay(100);

    int ssidChoice = Serial.parseInt();
    Serial.read(); // consume newline or leftover char

    if (ssidChoice == 0) {
      Serial.println("Rescanning networks...");
      continue;
    }

    if (ssidChoice < 1 || ssidChoice > numNetworks) {
      Serial.println("Invalid choice.");
      return;
    }

    String selectedSSID = WiFi.SSID(ssidChoice - 1);
    Serial.print("You selected: ");
    Serial.println(selectedSSID);

    String password = "";
    if (WiFi.encryptionType(ssidChoice - 1) != WIFI_AUTH_OPEN) {
      Serial.println("Enter password:");
      while (!Serial.available()) delay(100);
      password = Serial.readStringUntil('\n');
    }

    WiFi.begin(selectedSSID.c_str(), password.c_str());
    unsigned long startMillis = millis();

    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
      if (millis() - startMillis > 10000) {
        Serial.println("\nConnection failed. Retrying...");
        return;
      }
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n✅ Connected to WiFi!");
      Serial.print("📍 IP: ");
      Serial.println(WiFi.localIP());

      // 💾 Save new credentials
      setWiFiCredentials(selectedSSID.c_str(), password.c_str());
      saveConfig();
      break;
    } else {
      Serial.println("\n❌ Connection failed. Try another network.");
    }
  }
}



void connectAWS() {
    if (WiFi.status() != WL_CONNECTED) {
        connectToWiFi();
    }
    net.setCACert(AWS_CERT_CA);
    net.setCertificate(AWS_CERT_CRT);
    net.setPrivateKey(AWS_CERT_PRIVATE);
    client.setServer(AWS_IOT_ENDPOINT, 8883);

    Serial.println("Connecting to AWS IoT...");
    int retries = 0;
    while (!client.connected() && retries < 5) {
        if (client.connect(THINGNAME)) break;
        retries++;
        delay(1000);
        Serial.print(".");
    }

    if (!client.connected()) {
        Serial.println("AWS IoT Timeout!");
        ESP.restart();
    }
    Serial.println("AWS IoT Connected!");
    delay(500);  // ✅ Allow connection to stabilize before first publish
}

void publishMessage() {
    if (!client.connected()) return;

    StaticJsonDocument<256> doc;
    doc["deviceId"] = THINGNAME;
    doc["ppm_raw"] = sensorData.rawPPM;
    doc["ppm_corrected"] = sensorData.correctedPPM;
    doc["comp_factor"] = sensorData.compFactor;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;

    char buffer[512];
    serializeJson(doc, buffer);

    bool success = client.publish(AWS_IOT_PUBLISH_TOPIC, buffer);
    Serial.println(success ? "[MQTT] ✅ Publish Success" : "[MQTT] ❌ Publish Failed");
    Serial.println(buffer);
}



void updateShadow(PubSubClient &client, bool wifiState, bool mqttState, bool dhtState, bool mq6State) {
    String payload = "{";
    payload += "\"state\": { \"reported\": {";
    payload += "\"wifi\": \"" + String(wifiState ? "ON" : "OFF") + "\",";
    payload += "\"mqtt\": \"" + String(mqttState ? "ON" : "OFF") + "\",";
    payload += "\"dht11\": \"" + String(dhtState ? "ON" : "OFF") + "\",";
    payload += "\"mq6\": \"" + String(mq6State ? "ON" : "OFF") + "\",";
    payload += "\"temperature\": " + (isnan(lastTemp) ? "null" : String(lastTemp, 1)) + ",";
    payload += "\"humidity\": " + (isnan(lastHum) ? "null" : String(lastHum, 1)) + ",";
    payload += "\"mq6_level\": " + String(lastMQ6);
    payload += "} } }";

    client.publish(TOPIC_SHADOW_UPDATE, payload.c_str());
}

void taskWiFiMQTT(void *pvParameters) {
    connectAWS();
    vTaskDelete(NULL);
}

void taskBLE(void *pvParameters) {
    while (1) {
        char buf[16];
        dtostrf(currentGasPPM, 4, 2, buf);
        pGasCharacteristic->setValue(buf);
        pGasCharacteristic->notify();
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void taskPublish(void *pvParameters) {
    while (1) {
        if (!client.connected()) {
            reconnectAWS();  // 🛜 Will set justReconnected = true
        }

        if (justReconnected && millis() - lastReconnectTime < 2000) {
            client.loop();  // let TLS settle
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }

        Serial.printf("WiFi: %s | MQTT: %s | Heap: %d\n",
                      WiFi.isConnected() ? "ON" : "OFF",
                      client.connected() ? "ON" : "OFF",
                      ESP.getFreeHeap());

        publishMessage();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
void taskMQTTLoop(void *pvParameters) {
    while (1) {
        if (client.connected()) {
            client.loop();  // must run often to keep TLS session alive
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

