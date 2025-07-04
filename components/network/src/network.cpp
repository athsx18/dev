#include "network.h"
#include "config.h"
#include "globals.h"
#include "secrets.h"
#include "sensor.h"
#include "utils.h"
#include "alert.h"
#include <ArduinoJson.h>

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

void connectToWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    Serial.println(WiFi.localIP());
}

void connectAWS() {
    connectToWiFi();
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
}

void publishMessage() {
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
    Serial.println(success ? "[MQTT] Publish Success" : "[MQTT] Publish Failed");
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
        publishMessage();
        client.loop();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
