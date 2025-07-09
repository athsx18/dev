#include "input.h"
#include "config.h"

static ButtonCallback stopCallback = nullptr;
static ButtonCallback resetCallback = nullptr;

static unsigned long lastStopPress = 0;
static unsigned long lastResetPress = 0;
const unsigned long debounceDelay = 50;

void initInputSystem() {
    pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
}

void registerStopCallback(ButtonCallback cb) {
    stopCallback = cb;
}

void registerResetCallback(ButtonCallback cb) {
    resetCallback = cb;
}

void checkButtons() {
    unsigned long now = millis();

    // STOP Button Logic
    if (digitalRead(STOP_BUTTON_PIN) == LOW) {
        if ((now - lastStopPress) > debounceDelay) {
            if (stopCallback) stopCallback();
        }
        lastStopPress = now;
    }

    // RESET Button Logic
    if (digitalRead(RESET_BUTTON_PIN) == LOW) {
        if ((now - lastResetPress) > debounceDelay) {
            if (resetCallback) resetCallback();
        }
        lastResetPress = now;
    }
}
