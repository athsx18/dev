#ifndef SENSOR_H
#define SENSOR_H
#include "config_manager.h"

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
extern float danger;
extern float warning;
#define RL_VALUE              6.2
#define ADC_RESOLUTION        4095

#endif // SENSOR_H
