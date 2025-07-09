#ifndef ALERT_H
#define ALERT_H

void initAlertSystem();
void updateAlerts(float gasPPM);
void handleStopButton(float gasPPM);
void handleResetButton();
bool isAlertActive();
void performReset();
void taskAlertsAndButtons(void *pvParameters);


#endif
