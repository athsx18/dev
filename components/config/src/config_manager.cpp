#include "config_manager.h"
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#define CONFIG_FILE "/config.json"

static Config config;

// Load configuration from SPIFFS
bool loadConfig() {
    if (!SPIFFS.begin(true)) {
        Serial.println("❌ Failed to mount SPIFFS");
        return false;
    }

    if (!SPIFFS.exists(CONFIG_FILE)) {
        Serial.println("⚠️  Config file not found. Using defaults.");
        resetConfig();
        return false;
    }

    File file = SPIFFS.open(CONFIG_FILE, "r");
    if (!file || file.size() == 0) {
        Serial.println("⚠️  Empty or invalid config file. Using defaults.");
        file.close();
        resetConfig();
        return false;
    }

    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, file);
    file.close();

    if (err) {
        Serial.print("❌ Failed to parse config: ");
        Serial.println(err.c_str());
        resetConfig();
        return false;
    }

    config.dangerThreshold = doc["dangerThreshold"] | 500.0;
    config.warningThreshold = doc["warningThreshold"] | 200.0;
    config.calibrationRo = doc["calibrationRo"] | 10.0;

    strlcpy(config.wifiSSID, doc["wifiSSID"] | "", sizeof(config.wifiSSID));
    strlcpy(config.wifiPassword, doc["wifiPassword"] | "", sizeof(config.wifiPassword));

    Serial.println("✅ Config loaded from SPIFFS");
    Serial.printf("SSID: %s, PASS: %s\n", config.wifiSSID, config.wifiPassword);
    Serial.printf("Danger: %.2f, Warning: %.2f, Ro: %.2f\n",
                  config.dangerThreshold,
                  config.warningThreshold,
                  config.calibrationRo);

    return true;
}

// Save configuration to SPIFFS
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

    serializeJsonPretty(doc, file);
    file.close();
    Serial.println("✅ Config saved to SPIFFS");
    return true;
}

// Reset config to default values
void resetConfig() {
    config.dangerThreshold = 500.0;
    config.warningThreshold = 200.0;
    config.calibrationRo = 10.0;
    strlcpy(config.wifiSSID, "", sizeof(config.wifiSSID));
    strlcpy(config.wifiPassword, "", sizeof(config.wifiPassword));

    saveConfig();
    Serial.println("⚠️  Config reset to defaults");
}

// Get current config
Config getConfig() {
    return config;
}

// Setters for updating config
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
