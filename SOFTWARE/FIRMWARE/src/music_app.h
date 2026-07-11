#pragma once
#include "Audio.h"
// #include "BluetoothA2DPSource.h" // Disabled
// #include <freertos/ringbuf.h>

// External audio objects defined in main.cpp
extern Audio audio;
// extern BluetoothA2DPSource a2dp_source; // Disabled
// extern RingbufHandle_t audio_buffer;
// int32_t get_bluetooth_data(uint8_t *data, int32_t len);

enum AudioRoute { ROUTE_I2S /*, ROUTE_BT */ };
extern AudioRoute current_route;

void music_app_run();
