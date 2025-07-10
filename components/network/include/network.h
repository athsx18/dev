#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <PubSubClient.h>

// ---------- BLE Setup ----------
void setupBLE();

// ---------- Wi-Fi and AWS IoT ----------
void connectToWiFi();
void connectAWS();
void publishMessage();
void updateShadow(PubSubClient &client, bool wifiState, bool mqttState, bool dhtState, bool mq6State);

// ---------- Network Tasks ----------
void taskWiFiMQTT(void *pvParameters);
void taskBLE(void *pvParameters);
void taskPublish(void *pvParameters);
void setupAWSMQTTClient();
void taskMQTTLoop(void *pvParameters);

#endif // NETWORK_H
