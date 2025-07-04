// sensor.cpp
#include "sensor.h"
#include "globals.h"
#include "utils.h"
#include <DHT.h>
#include "network.h"

extern DHT dht;

float calibrateMQ6(int pin) {
    lcd.clear();
    lcd.print("Calibrating MQ6");
    delay(5000);

    long totalReadings = 0;
    for (int i = 0; i < 500; i++) {
        int rawValue = analogRead(pin);
        float voltage = rawValue * (3.3 / 4095.0);
        float sensorResistance = ((3.3 * RL_VALUE) / voltage) - RL_VALUE;
        totalReadings += sensorResistance;
        delay(10);
    }
    float calibrationValue = totalReadings / 500.0;
    lcd.clear();
    lcd.print("Calib Done");
    lcd.setCursor(0, 1);
    lcd.print(calibrationValue);
    delay(2000);
    return calibrationValue;
}

float readMQ6PPM(int pin, float ro) {
    int rawValue = analogRead(pin);
    float voltage = rawValue * (3.3 / ADC_RESOLUTION);
    float rs = ((3.3 * RL_VALUE) / voltage) - RL_VALUE;
    float a = 2.3;
    float b = -0.47;
    return pow((rs / ro) / a, 1 / b);
}

void taskDHT(void *pvParameters) {
    dht.begin();
    while (true) {
        float t = dht.readTemperature();
        float h = dht.readHumidity();
        if (!isnan(t) && !isnan(h)) {
            temperature = t;
            humidity = h;
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}


void taskGasSensor(void *pvParameters) {
    while (true) {
        float rawPPM = readMQ6PPM(MQ5_PIN, Ro);
        float compFactor = getCompensationFactor(temperature, humidity, rawPPM);
        float correctedPPM = rawPPM * compFactor;

        currentGasPPM = correctedPPM;
        sensorData.rawPPM = rawPPM;
        sensorData.correctedPPM = correctedPPM;
        sensorData.compFactor = compFactor;

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void taskReadSensors(void *pvParameters) {
  while (true) {
    bool wifiState = (WiFi.status() == WL_CONNECTED);
    bool mqttState = client.connected();

    lastTemp = temperature;
    lastHum = humidity;

    bool dhtState = !isnan(lastTemp) && !isnan(lastHum);

    lastMQ6 = sensorData.correctedPPM;  // or currentGasPPM

    bool mq6State = lastMQ6 > 0;

    updateShadow(client, wifiState, mqttState, dhtState, mq6State);
    vTaskDelay(5000 / portTICK_PERIOD_MS);  // every 5 seconds
  }
}