#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

void initDisplay();                        // call from setup()
void showStatus(const String& message);    // generic status
void showReading(float ppm);               // formatted gas reading
void clearDisplay();                       // clears LCD
void taskLCD(void* pvParameters);          // LCD FreeRTOS task

#endif
