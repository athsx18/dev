#ifndef CONFIG_H
#define CONFIG_H

// ---------- Pin Definitions ----------
#define MQ5_PIN           34
#define GREEN_LED_PIN     32
#define YELLOW_LED_PIN    14
#define RED_LED_PIN       33
#define BUZZER_PIN        25
#define STOP_BUTTON_PIN   26
#define RESET_BUTTON_PIN  27

// ---------- BLE UUIDs ----------
#define SERVICE_UUID         "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID  "abcd1234-5678-90ab-cdef-1234567890ab"

// ---------- AWS Topics ----------
#define AWS_IOT_PUBLISH_TOPIC "esp32/pub"
#define TOPIC_SHADOW_UPDATE   "$aws/things/TeezSense1/shadow/update"
#define CLIENT_ID             "TeezSense1"

#endif // CONFIG_H
