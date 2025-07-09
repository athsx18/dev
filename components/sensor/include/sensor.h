#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

// ---------- Public API ----------
void initializeMQ6(int pin);
void calibrate();
float readConcentration();

// ---------- DHT Sensor ----------
void taskDHT(void *pvParameters);
void taskReadSensors(void *pvParameters);

// ---------- Gas Sensor Task ----------
void taskGasSensor(void *pvParameters);

// ---------- Compensation ----------
float getCompensationFactor(float T, float H, float raw);

// ---------- Thresholds ----------
#define WARNING_THRESHOLD     200.0
#define DANGER_THRESHOLD      400.0
#define RL_VALUE              6.2
#define ADC_RESOLUTION        4095

#endif // SENSOR_H
