#include "sensor.h"
#include "globals.h"
#include "utils.h"
#include <DHT.h>
#include "network.h"

static int mq6Pin;
static float Ro = 10.0;


// Initialization
void initializeMQ6(int pin) {
    mq6Pin = pin;
}

// Calibration
void calibrate() {
    lcd.clear();
    lcd.print("Calibrating MQ6");
    delay(5000);

    long totalReadings = 0;
    for (int i = 0; i < 500; i++) {
        int rawValue = analogRead(mq6Pin);
        float voltage = rawValue * (3.3 / ADC_RESOLUTION);
        float rs = ((3.3 * RL_VALUE) / voltage) - RL_VALUE;
        totalReadings += rs;
        delay(10);
    }
    Ro = totalReadings / 500.0;

    lcd.clear();
    lcd.print("Calib Done");
    lcd.setCursor(0, 1);
    lcd.print(Ro);
    delay(2000);
}

// Read & calculate corrected PPM
float readConcentration() {
    int rawValue = analogRead(mq6Pin);
    float voltage = rawValue * (3.3 / ADC_RESOLUTION);
    float rs = ((3.3 * RL_VALUE) / voltage) - RL_VALUE;

    float a = 2.3;
    float b = -0.47;
    float rawPPM = pow((rs / Ro) / a, 1 / b);
    float compFactor = getCompensationFactor(temperature, humidity, rawPPM);
    float correctedPPM = rawPPM * compFactor;

    sensorData.rawPPM = rawPPM;
    sensorData.correctedPPM = correctedPPM;
    sensorData.compFactor = compFactor;

    currentGasPPM = correctedPPM;
    return correctedPPM;
}

// Gas sensor task
void taskGasSensor(void *pvParameters) {
    while (true) {
        readConcentration();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// DHT task
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

// Read sensor status to update shadow
void taskReadSensors(void *pvParameters) {
    while (true) {
        bool wifiState = (WiFi.status() == WL_CONNECTED);
        bool mqttState = client.connected();

        lastTemp = temperature;
        lastHum = humidity;
        lastMQ6 = sensorData.correctedPPM;

        bool dhtState = !isnan(lastTemp) && !isnan(lastHum);
        bool mq6State = lastMQ6 > 0;

        updateShadow(client, wifiState, mqttState, dhtState, mq6State);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
