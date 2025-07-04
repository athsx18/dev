#ifndef ALERT_H
#define ALERT_H

#include <Arduino.h>

// ---------- Alert Task ----------
void taskAlertsAndButtons(void *pvParameters);

void taskLCD(void *pvParameters);  // ✅ Ensure this is declared

// ---------- LCD Status ----------
extern int lcdStatus;

// ---------- Alert Functions ----------
void updateAlerts(float gas_ppm);
void performReset();
void handleButton();
void handleResetButton();

#endif // ALERT_H
