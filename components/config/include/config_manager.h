#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

struct Config {
    float dangerThreshold;
    float warningThreshold;
    float calibrationRo;
    char wifiSSID[32];
    char wifiPassword[64];
};

// Load config from SPIFFS `/config.json`
bool loadConfig();

// Save current config to SPIFFS
bool saveConfig();

// Reset to default values and save
void resetConfig();

// Get current config
Config getConfig();

// Setters
void setDangerThreshold(float val);
void setWarningThreshold(float val);
void setCalibrationRo(float val);
void setWiFiCredentials(const char* ssid, const char* password);

#endif
