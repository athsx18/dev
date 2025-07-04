#ifndef ALERT_H
#define ALERT_H

void updateAlerts(float gas_ppm);
void performReset();
void handleButton();
void handleResetButton();
void taskAlertsAndButtons(void *pvParameters);

#endif
