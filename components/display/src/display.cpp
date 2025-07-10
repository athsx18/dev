#include "display.h"
#include "globals.h"
#include "config.h"
#include "sensor.h"

void initDisplay() {
    lcdStatus = lcd.begin(16, 2);
    if (lcdStatus) {
        Serial.print("LCD init failed: ");
        Serial.println(lcdStatus);
        while (1);
    }
    lcd.setBacklight(true);
    clearDisplay();
    showStatus("WiFi Connecting");
}

void clearDisplay() {
    lcd.clear();
}

void showStatus(const String& message) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message);
}

void showReading(float ppm) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Gas: ");
    lcd.print(ppm, 2);
    lcd.print(" PPM");

    lcd.setCursor(0, 1);
    if (ppm <= getConfig().warningThreshold) {
        lcd.print("Safe Level");
    } else if (ppm <= getConfig().dangerThreshold) {
        lcd.print("Critical Level");
    } else {
        lcd.print("Danger Level");
    }
}

void taskLCD(void* pvParameters) {
    while (1) {
        showReading(currentGasPPM);  // Always show on LCD

        if (client.connected()) {
            Serial.printf("Gas: %.2f PPM\n", currentGasPPM);  // Only print if MQTT connected
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

