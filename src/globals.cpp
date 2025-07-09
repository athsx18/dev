#include "globals.h"
#include <DHT.h>


DHT dht(4, DHT11); 

// ---------- Global Sensor Variables ----------
//float Ro = 10.0;
float currentGasPPM = 0.0;
float temperature = 0.0;
float humidity = 0.0;
float lastTemp = 0.0;
float lastHum = 0.0;
float lastMQ6 = 0.0;
SensorData sensorData;

// ---------- Flags & Timers ----------
bool buzzerStopped = false;
bool resetTriggered = false;
bool resetCompleteDisplayed = false;
unsigned long resetCompleteTime = 0;

// ---------- LCD ----------
hd44780_I2Cexp lcd;
int lcdStatus = 0;

// ---------- Wi-Fi & MQTT ----------
WiFiClientSecure net;
PubSubClient client(net);

// ---------- BLE ----------
NimBLEServer* pServer = nullptr;
NimBLECharacteristic* pGasCharacteristic = nullptr;
