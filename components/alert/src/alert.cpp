#include "alert.h"
#include "config.h"
#include "globals.h"
#include "sensor.h"
#include "network.h"

void updateAlerts(float gas_ppm) {
    if (gas_ppm > DANGER_THRESHOLD) {
        if (!buzzerStopped) {
            digitalWrite(RED_LED_PIN, HIGH);
            digitalWrite(YELLOW_LED_PIN, LOW);
            digitalWrite(GREEN_LED_PIN, LOW);
            ledcWrite(0, 128);
        }
    } else if (gas_ppm > WARNING_THRESHOLD) {
        digitalWrite(RED_LED_PIN, LOW);
        digitalWrite(YELLOW_LED_PIN, HIGH);
        digitalWrite(GREEN_LED_PIN, LOW);
        ledcWrite(0, 0);
    } else {
        digitalWrite(RED_LED_PIN, LOW);
        digitalWrite(YELLOW_LED_PIN, LOW);
        digitalWrite(GREEN_LED_PIN, HIGH);
        ledcWrite(0, 0);
    }

    if (gas_ppm <= DANGER_THRESHOLD) {
        buzzerStopped = false;
    }
}

void performReset() {
    lcd.clear(); lcd.print("Resetting...");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF); delay(1000);
    WiFi.mode(WIFI_STA);

    connectToWiFi();
    connectAWS();
    Ro = calibrateMQ6(MQ5_PIN);

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

void handleButton() {
    static unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 50;
    if (digitalRead(STOP_BUTTON_PIN) == LOW && currentGasPPM > DANGER_THRESHOLD) {
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

void taskAlertsAndButtons(void *pvParameters) {
    while (1) {
        handleButton();
        handleResetButton();
        updateAlerts(currentGasPPM);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}
