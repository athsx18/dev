#include "display.h"
#include "globals.h"
#include "sensor.h"

void taskLCD(void *pvParameters) {
    while (1) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Gas: ");
        lcd.print(currentGasPPM, 2);
        lcd.print(" PPM");

        lcd.setCursor(0, 1);
        if (currentGasPPM <= WARNING_THRESHOLD) {
            lcd.print("Safe Level");
        } else if (currentGasPPM <= DANGER_THRESHOLD) {
            lcd.print("Critical Level");
        } else {
            lcd.print("Danger Level");
        }

        Serial.printf("Gas: %.2f PPM\n", currentGasPPM);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
