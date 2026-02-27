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

// App launch callbacks
void launch_settings() {
    Serial.println("Launching Settings...");
    settings_run();
}

void launch_music() {
    Serial.println("Launching Music...");
    // Placeholder for music app
}

void launch_messages() {
    Serial.println("Launching Messages...");
    // Placeholder for messages app
}

void setup(){
  Serial.begin(115200);
  
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
  // Assumes a voltage divider reading roughly 3.3V-4.2V scale as top percent
  int bat_raw = analogRead(14);
  int bat_level = 0;
  if(bat_raw > 1800) bat_level = 1;
  if(bat_raw > 2200) bat_level = 2;
  if(bat_raw > 2400) bat_level = 3;
  if(bat_raw > 2500) bat_level = 4;
  
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