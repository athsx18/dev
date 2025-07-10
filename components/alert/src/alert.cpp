#include <Arduino.h>
#include "alert.h"
#include "config.h"
#include "network.h"
#include "sensor.h"
#include "display.h"
#include "globals.h"
#include "input.h"
#include "config_manager.h"

static bool buzzerStopped = false;
static bool alertActive = false;
static bool resetTriggered = false;
static unsigned long resetCompleteTime = 0;
static bool resetCompleteDisplayed = false;

void onStopPressed() {
    buzzerStopped = true;
    ledcWrite(0, 0);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
}

void onResetPressed() {
    performReset();
}

void initAlertSystem() {
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(YELLOW_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

    ledcSetup(0, 2000, 8); // Channel 0, 2kHz freq, 8-bit resolution
    ledcAttachPin(BUZZER_PIN, 0);

    initInputSystem();                     // 👈 INIT input module
    registerStopCallback(onStopPressed);  // 👈 REGISTER callback
    registerResetCallback(onResetPressed);
}

void updateAlerts(float gasPPM) {
    if (gasPPM > getConfig().dangerThreshold) {
        alertActive = true;
        if (!buzzerStopped) {
            digitalWrite(RED_LED_PIN, HIGH);
            digitalWrite(YELLOW_LED_PIN, LOW);
            digitalWrite(GREEN_LED_PIN, LOW);
            ledcWrite(0, 128); // Buzzer on
        }
    } else if (gasPPM > getConfig().warningThreshold) {
        alertActive = true;
        digitalWrite(RED_LED_PIN, LOW);
        digitalWrite(YELLOW_LED_PIN, HIGH);
        digitalWrite(GREEN_LED_PIN, LOW);
        ledcWrite(0, 0); // Buzzer off
    } else {
        alertActive = false;
        digitalWrite(RED_LED_PIN, LOW);
        digitalWrite(YELLOW_LED_PIN, LOW);
        digitalWrite(GREEN_LED_PIN, HIGH);
        ledcWrite(0, 0); // Buzzer off
    }

    if (gasPPM <= getConfig().dangerThreshold) {
        buzzerStopped = false;
    }
}


void performReset() {
    lcd.clear();
    lcd.print("Resetting...");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF); delay(1000);
    WiFi.mode(WIFI_STA);

    connectToWiFi();
    connectAWS();
    initializeMQ6(MQ6_PIN);
    calibrate();

    buzzerStopped = false;
    resetTriggered = true;
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
    ledcWrite(0, 0);
    resetCompleteTime = millis();
    resetCompleteDisplayed = true;
    resetTriggered = false;
}

bool isAlertActive() {
    return alertActive;
}

void taskAlertsAndButtons(void *pvParameters) {
    while (1) {
        checkButtons();                       // 👈 input module handles STOP + RESET
        updateAlerts(currentGasPPM);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

