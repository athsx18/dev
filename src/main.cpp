#include <Arduino.h>
#include "network.h"
#include "sensor.h"
#include "display.h"
#include "alert.h"
#include "config.h"
#include "globals.h"

void setup() {
    Serial.begin(115200);

    /* lcdStatus = lcd.begin(16, 2);
    if (lcdStatus) {
        Serial.print("LCD init failed: "); Serial.println(lcdStatus);
        while (1);
    }
    lcd.setBacklight(true);
    lcd.clear(); lcd.setCursor(0, 0); lcd.print("Connecting WiFi");*/

    // Wi-Fi + MQTT + LCD
    connectAWS();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connected!");
    delay(1000);

    // MQ6 Initialization & Calibration
    initializeMQ6(MQ6_PIN);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Calibrating MQ6...");
    calibrate();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("MQ6 Gas Monitor");
    delay(2000);

    // BLE Setup
    setupBLE();

    // I/O Pins
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(YELLOW_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

    // Buzzer PWM setup
    ledcSetup(0, 2000, 8);
    ledcAttachPin(BUZZER_PIN, 0);

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
