#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>

// Callbacks
typedef void (*ButtonCallback)();

void initInputSystem();  // Set pinModes
void checkButtons();     // Call from task or loop

// Register callbacks for STOP and RESET button
void registerStopCallback(ButtonCallback cb);
void registerResetCallback(ButtonCallback cb);

#endif
