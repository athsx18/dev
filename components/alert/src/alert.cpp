#include <Arduino.h>
#include "alert.h"
#include "config.h"
#include "network.h"
#include "sensor.h"
#include "display.h"
#include "globals.h"

static bool buzzerStopped = false;
static bool alertActive = false;
static bool resetTriggered = false;
static unsigned long resetCompleteTime = 0;
static bool resetCompleteDisplayed = false;

void initAlertSystem() {
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(YELLOW_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

    ledcSetup(0, 2000, 8); // Channel 0, 2kHz freq, 8-bit resolution
    ledcAttachPin(BUZZER_PIN, 0);
}

void updateAlerts(float gasPPM) {
    if (gasPPM > DANGER_THRESHOLD) {
        alertActive = true;
        if (!buzzerStopped) {
            digitalWrite(RED_LED_PIN, HIGH);
            digitalWrite(YELLOW_LED_PIN, LOW);
            digitalWrite(GREEN_LED_PIN, LOW);
            ledcWrite(0, 128); // Buzzer on
        }
    } else if (gasPPM > WARNING_THRESHOLD) {
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

    if (gasPPM <= DANGER_THRESHOLD) {
        buzzerStopped = false;
    }
}

void handleStopButton(float gasPPM) {
    static unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 50;

    if (digitalRead(STOP_BUTTON_PIN) == LOW && gasPPM > DANGER_THRESHOLD) {
        unsigned long currentTime = millis();
        if ((currentTime - lastDebounceTime) > debounceDelay) {
            buzzerStopped = true;
            ledcWrite(0, 0);
            digitalWrite(RED_LED_PIN, LOW);
            digitalWrite(YELLOW_LED_PIN, LOW);
            digitalWrite(GREEN_LED_PIN, HIGH);
        }
        lastDebounceTime = currentTime;
    }
}

void handleResetButton() {
    static unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 50;

    if (digitalRead(RESET_BUTTON_PIN) == LOW) {
        unsigned long currentTime = millis();
        if ((currentTime - lastDebounceTime) > debounceDelay) {
            performReset();
        }
        lastDebounceTime = currentTime;
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
        handleStopButton(currentGasPPM);
        handleResetButton();
        updateAlerts(currentGasPPM);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}
