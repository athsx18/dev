#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

// ---------- MQ6 Sensor ----------
float calibrateMQ6(int pin);
float readMQ6PPM(int pin, float ro);

// ---------- DHT Sensor ----------
void taskDHT(void *pvParameters);

// ---------- MQ6 Compensation ----------
float getCompensationFactor(float T, float H, float raw);

// ---------- Gas Sensor Task ----------
void taskGasSensor(void *pvParameters);
void taskReadSensors(void *pvParameters);

// ---------- Gas Thresholds ----------
#define WARNING_THRESHOLD     200.0
#define DANGER_THRESHOLD      400.0
#define RL_VALUE              6.2    // kΩ
#define ADC_RESOLUTION        4095
#define CLEAN_AIR_RATIO       6.5



#endif // SENSOR_H
