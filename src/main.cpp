#include <Arduino.h>
#include "network.h"
#include "sensor.h"
#include "display.h"
#include "alert.h"
#include "config.h"
#include "globals.h"

void setup() {
    Serial.begin(115200);

    // LCD Module
    initDisplay();
    showStatus("WiFi Connecting");

    // Wi-Fi + MQTT
    connectAWS();
    showStatus("Connected!");
    delay(1000);

    // MQ6 Initialization & Calibration
    initializeMQ6(MQ6_PIN);
    showStatus("Calibrating MQ6...");
    calibrate();
    showStatus("MQ6 Gas Monitor");
    delay(2000);

    // BLE Setup
    setupBLE();

    // ALERT SYSTEM (LEDs, Buzzer, Buttons)
    initAlertSystem();

    // FreeRTOS Tasks
    xTaskCreate(taskAlertsAndButtons, "AlertsButtons", 8192, NULL, 4, NULL);
    xTaskCreate(taskGasSensor, "GasSensor", 4096, NULL, 3, NULL);
    xTaskCreate(taskPublish, "Publish", 4096, NULL, 2, NULL);
    xTaskCreate(taskLCD, "LCD", 4096, NULL, 1, NULL);
    xTaskCreate(taskBLE, "BLE", 4096, NULL, 1, NULL);
    xTaskCreate(taskDHT, "DHT", 2048, NULL, 1, NULL);
    xTaskCreate(taskReadSensors, "SensorState", 4096, NULL, 1, NULL);
}

void loop() {
    
}
