#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <time.h>
#include "esp_bt.h"
#include <services.h>
#include <servicemgr.h>
#include <phone_gui.h>
#include <keypad_driver.h>
#include "settings_app.h"
#include "music_app.h"
#include "Audio.h"
// #include "BluetoothA2DPSource.h" // Disabled
#include <freertos/ringbuf.h>

// Audio routing state and buffers
AudioRoute current_route = ROUTE_I2S;
RingbufHandle_t audio_buffer = NULL;

Audio audio;
// BluetoothA2DPSource a2dp_source; // Disabled

// Optional internal speaker pins (User can adjust later)
#define I2S_LRC      4
#define I2S_BCLK     5
#define I2S_DOUT     6
#define I2S_SD_MODE  3  // MAX98357A Enable/Channel Select

service_status_t log_buf;
uint8_t selected_icon = 0;

// Define App structure for scalable main menu
typedef struct {
    const char* name;
    const unsigned char* icon_full;
    const unsigned char* icon_small;
    void (*onLaunch)();
} App;

// Forward declarations for app launch callbacks
void launch_settings();
void launch_music();
void launch_messages();

// Array of apps
App apps[] = {
    {"SETTINGS", image_menu_tools_full_bits, image_menu_tools_small_bits, launch_settings},
    {"MUSIC", image_music_radio_full_bits, image_music_radio_small_bits, launch_music},
    {"MESSAGES", image_message_mail_full_bits, image_message_mail_small_bits, launch_messages}
};

extern "C" void service_log_print_serial(const char *msg) {
    if (Serial) {
        Serial.print(msg);
    }
}

const uint8_t NUM_APPS = sizeof(apps) / sizeof(apps[0]);

// FreeRTOS RingBuffer size (bytes)
#define AUDIO_BUFFER_SIZE 16384

// ESP32-audioI2S weak callback override
/*
void audio_process_i2s(int32_t* outBuff, int16_t validSamples, bool* continueI2S) {
  // Disabled
}

int32_t get_bluetooth_data(uint8_t *data, int32_t len) {
  // Disabled
  return 0;
}
*/

// App launch callbacks
void launch_settings() {
    Serial.println("Launching Settings...");
    settings_run();
}

void launch_music() {
    Serial.println("Launching Music...");
    music_app_run();
}

void launch_messages() {
    Serial.println("Launching Messages...");
    // Placeholder for messages app
}

void setup(){
  Serial.begin(115200);

  // Setup I2S Pins
  pinMode(I2S_SD_MODE, OUTPUT);
  digitalWrite(I2S_SD_MODE, HIGH); // Enable MAX98357A (Left Channel Only if +3.3V, Mix if >1.4V? depends on wiring)
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(15); // Start with moderate volume

  // Init ring buffer for BT audio bridge
  audio_buffer = xRingbufferCreate(AUDIO_BUFFER_SIZE, RINGBUF_TYPE_BYTEBUF);
/*
  if (audio_buffer) {
    a2dp_source.set_data_callback(get_bluetooth_data);
  }
*/
  
  // Start services
  service_start("display");  
  service_start("keyboard");
  service_start("wifi");

  // Configure Time (SNTP) once WiFi is up
  // Set to simple European timezone as placeholder
  configTime(2 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  // Configure battery ADC (Pin 14)
  pinMode(14, INPUT);
  // Initialize settings (loads contrast, etc.)
  settings_init();

  delay(100);
}

void run_main_menu() {
  // Simple test loop
  char key = keypad.getKey();
  if (key) {
    Serial.print("Key Pressed: ");
    Serial.println(key);
    
    // Increment on 'R' (Right), Decrement on 'L' (Left)
    if (key == 'R') {
      selected_icon = (selected_icon + 1) % NUM_APPS;
    } else if (key == 'L') {
      if (selected_icon == 0) selected_icon = NUM_APPS - 1; // Wrap around
      else selected_icon--;
    } else if (key == 'O') {
      // Launch selected app
      if (apps[selected_icon].onLaunch) {
          apps[selected_icon].onLaunch();
      }
    }
    
    // Prevent rapid scrolling if the key is held down (simple debounce/delay)
    delay(200); 
  }
  
  // Calculate left and right indices for the menu
  uint8_t left_icon = (selected_icon == 0) ? NUM_APPS - 1 : selected_icon - 1;
  uint8_t right_icon = (selected_icon + 1) % NUM_APPS;

  // Sync Hardware state
  bool bt_on = settings_get_bt();
  if (bt_on && !esp_bt_controller_get_status()) {
     // Optional: You could write a bluetooth_service wrapper.
     // For now this effectively signals intent.
  }
  
  // Read battery ADC and map 0-4095 roughly to 0-4 scale.
  // 1/1 Divider means Pin Voltage = Vbat / 2.
  // Full (4.2V) -> 2.1V at pin. Empty (3.3V) -> 1.65V at pin.
  // ESP32 ADC (11dB) maps ~3.1V to 4095.
  // 2.1V  => ~2770
  // 1.9V  => ~2500
  // 1.8V  => ~2370
  // 1.7V  => ~2240
  // 1.65V => ~2170
  int bat_raw = analogRead(14);
  int bat_level = 0;
  if(bat_raw > 2170) bat_level = 1; // > 3.3V
  if(bat_raw > 2370) bat_level = 2; // > 3.6V
  if(bat_raw > 2500) bat_level = 3; // > 3.8V
  if(bat_raw > 2650) bat_level = 4; // > 4.0V
  
  // Get time from NTP (updates when wifi is connected over SNTP)
  struct tm timeinfo;
  char time_str[10] = "--:--";
  if (getLocalTime(&timeinfo, 10)) {
     strftime(time_str, sizeof(time_str), "%H:%M", &timeinfo);
  }

  // Draw current selection
  gui.drawMainMenu(bat_level, bt_on ? 1 : 0, 0, time_str,
                   apps[selected_icon].name, 
                   apps[selected_icon].icon_full, 
                   apps[left_icon].icon_small, 
                   apps[right_icon].icon_small,
                   selected_icon + 1,
                   NUM_APPS);
}

void loop(){
  run_main_menu();
  delay(50); 
}