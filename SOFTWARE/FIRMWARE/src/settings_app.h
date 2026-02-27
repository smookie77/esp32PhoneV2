#ifndef SETTINGS_APP_H
#define SETTINGS_APP_H

#include <Arduino.h>

// Initialize settings (load from flash, init SD, etc.)
void settings_init();

// Run the settings app loop. Returns when the user exits.
void settings_run();

// Access settings logic without UI
bool settings_get_bt();

#endif
