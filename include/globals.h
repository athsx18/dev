#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <Wire.h>  // ✅ Must come before hd44780_I2Cexp
#include <PubSubClient.h>
#include <DHT.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <NimBLEDevice.h>
#include "config.h"
#include "types.h"
extern DHT dht;

// ---------- Global Sensor Variables ----------
//extern float Ro;
extern float currentGasPPM;
extern float temperature;
extern float humidity;
extern float lastTemp;
extern float lastHum;
extern float lastMQ6;
extern SensorData sensorData;

// ---------- Flags & Timers ----------
extern bool buzzerStopped;
extern bool resetTriggered;
extern bool resetCompleteDisplayed;
extern unsigned long resetCompleteTime;

// ---------- LCD ----------
extern hd44780_I2Cexp lcd;
extern int lcdStatus;

// ---------- Wi-Fi & MQTT ----------
extern WiFiClientSecure net;
extern PubSubClient client;

// ---------- BLE ----------
extern NimBLEServer* pServer;
extern NimBLECharacteristic* pGasCharacteristic;

#endif // GLOBALS_H
