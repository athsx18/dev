#include "config_manager.h"
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#define CONFIG_FILE "/config.json"

static Config config;

bool loadConfig() {
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        return false;
    }

    File file = SPIFFS.open(CONFIG_FILE, "r");
    if (!file) {
        Serial.println("Config file not found. Using defaults.");
        resetConfig();
        return false;
    }

    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, file);
    file.close();

    if (err) {
        Serial.println("Failed to parse config. Using defaults.");
        resetConfig();
        return false;
    }

    config.dangerThreshold = doc["dangerThreshold"] | 500.0;
    config.warningThreshold = doc["warningThreshold"] | 200.0;
    config.calibrationRo = doc["calibrationRo"] | 10.0;
    strlcpy(config.wifiSSID, doc["wifiSSID"] | "defaultSSID", sizeof(config.wifiSSID));
    strlcpy(config.wifiPassword, doc["wifiPassword"] | "defaultPASS", sizeof(config.wifiPassword));

    Serial.println("✅ Config loaded from SPIFFS");
    return true;
}

bool saveConfig() {
    StaticJsonDocument<512> doc;
    doc["dangerThreshold"] = config.dangerThreshold;
    doc["warningThreshold"] = config.warningThreshold;
    doc["calibrationRo"] = config.calibrationRo;
    doc["wifiSSID"] = config.wifiSSID;
    doc["wifiPassword"] = config.wifiPassword;

    File file = SPIFFS.open(CONFIG_FILE, "w");
    if (!file) {
        Serial.println("❌ Failed to open config file for writing.");
        return false;
    }

    serializeJson(doc, file);
    file.close();
    Serial.println("✅ Config saved to SPIFFS");
    return true;
}

void resetConfig() {
    config.dangerThreshold = 500.0;
    config.warningThreshold = 200.0;
    config.calibrationRo = 10.0;
    strlcpy(config.wifiSSID, "defaultSSID", sizeof(config.wifiSSID));
    strlcpy(config.wifiPassword, "defaultPASS", sizeof(config.wifiPassword));
    saveConfig();
    Serial.println("⚠️  Config reset to defaults");
}

Config getConfig() {
    return config;
}

void setDangerThreshold(float val) {
    config.dangerThreshold = val;
}

void setWarningThreshold(float val) {
    config.warningThreshold = val;
}

void setCalibrationRo(float val) {
    config.calibrationRo = val;
}

void setWiFiCredentials(const char* ssid, const char* password) {
    strlcpy(config.wifiSSID, ssid, sizeof(config.wifiSSID));
    strlcpy(config.wifiPassword, password, sizeof(config.wifiPassword));
}
