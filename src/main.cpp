#include <Arduino.h>
#include "network.h"
#include "sensor.h"
#include "display.h"
#include "alert.h"
#include "config.h"
#include "globals.h"
#include "config_manager.h"

void setup() {
    Serial.begin(115200);
    loadConfig();                   // Load thresholds, Ro, SSID
    showStatus("WiFi Connecting");

    connectToWiFi();               // ✅ Connect Wi-Fi
    setupAWSMQTTClient();          // ✅ One-time cert & MQTT config
    showStatus("Connected!");
    delay(1000);

    initializeMQ6(MQ6_PIN);        // Gas Sensor Setup
    showStatus("Calibrating MQ6...");
    calibrate();
    showStatus("MQ6 Gas Monitor");
    delay(2000);

    setupBLE();                    // BLE Init
    initAlertSystem();             // LED, Buzzer, Button GPIO

    // 🧵 Start FreeRTOS Tasks
    xTaskCreate(taskAlertsAndButtons, "AlertsButtons", 8192, NULL, 4, NULL);
    xTaskCreate(taskGasSensor, "GasSensor", 4096, NULL, 3, NULL);
    xTaskCreate(taskPublish, "Publish", 8192, NULL, 2, NULL);
    xTaskCreate(taskMQTTLoop, "MQTT Loop", 4096, NULL, 2, NULL);  // ✅ Required
    xTaskCreate(taskLCD, "LCD", 4096, NULL, 1, NULL);
    xTaskCreate(taskBLE, "BLE", 4096, NULL, 1, NULL);
    xTaskCreate(taskDHT, "DHT", 2048, NULL, 1, NULL);
    xTaskCreate(taskReadSensors, "SensorState", 4096, NULL, 1, NULL);
}


void loop() {
    
}
