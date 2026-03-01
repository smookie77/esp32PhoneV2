#include "settings_app.h"
#include "phone_gui.h"
#include "keypad_driver.h"
// #include "BluetoothA2DPSource.h" // Disabled
#include "music_app.h"
// #include "esp_bt_defs.h"
#include <Preferences.h>
#include <WiFi.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <vector>

// Global state for settings
static Preferences prefs;
static uint8_t current_contrast = 200;
static bool bluetooth_enabled = false;
static bool sd_initialized = false;
static String bt_last_name;
static std::vector<String> bt_scan_results;
static bool bt_scanning = false;
static uint32_t bt_scan_start_ms = 0;
// extern BluetoothA2DPSource a2dp_source; // Disabled

// Forward declarations for setting callbacks
static void on_wifi_select();
static void on_sd_select();
static void on_contrast_left();
static void on_contrast_right();
// static void on_bt_select(); // Disabled
static void on_back_select();
// static bool bt_scan_cb(const char* name, esp_bd_addr_t address, int rssi); // Disabled

// Struct for a setting item
typedef struct {
    const char* name;
    void (*onSelect)();
    void (*onLeft)();
    void (*onRight)();
    void (*onDraw)(char* buffer, size_t len); // Optional: to format the display string dynamically
} SettingItem;

// Dynamic draw functions
static void draw_contrast(char* buffer, size_t len) {
    snprintf(buffer, len, "Contrast: %d", current_contrast);
}

/*
static void draw_bt(char* buffer, size_t len) {
    if (bt_last_name.length() > 0) {
        snprintf(buffer, len, "BT: %s", bt_last_name.c_str());
    } else {
        snprintf(buffer, len, "Bluetooth: %s", bluetooth_enabled ? "ON" : "OFF");
    }
}
*/

// The list of settings
static SettingItem settings[] = {
    {"WiFi Info", on_wifi_select, nullptr, nullptr, nullptr},
    {"SD Card Info", on_sd_select, nullptr, nullptr, nullptr},
    {"Contrast", nullptr, on_contrast_left, on_contrast_right, draw_contrast},
    // {"Bluetooth", on_bt_select, nullptr, nullptr, draw_bt}, // Disabled
    {"Back", on_back_select, nullptr, nullptr, nullptr}
};

static const uint8_t NUM_SETTINGS = sizeof(settings) / sizeof(settings[0]);
static uint8_t selected_setting = 0;
static bool exit_settings = false;

// --- Callbacks ---

static void on_wifi_select() {
    gui.clear();
    gui.drawListMenu("WiFi Info", nullptr, 0, 0); // Just draw title
    
    char buf[32];
    if (WiFi.status() == WL_CONNECTED) {
        snprintf(buf, sizeof(buf), "SSID: %s", WiFi.SSID().c_str());
        String ipStr = WiFi.localIP().toString();
        const char* items[] = {buf, ipStr.c_str(), "Press O to return"};
        gui.drawListMenu("WiFi Info", items, 3, 2);
    } else {
        const char* items[] = {"Not Connected", "Press O to return"};
        gui.drawListMenu("WiFi Info", items, 2, 1);
    }
    
    // Wait for 'O' to return
    while (true) {
        char key = keypad.getKey();
        if (key == 'O') {
            break;
        } else if (key == 'E') {
            exit_settings = true;
            break;
        }
        delay(50);
    }
}

static void on_sd_select() {
    gui.clear();
    if (!sd_initialized) {
        // Try to initialize SD card with custom SPI pins
        SPI.begin(2, 44, 43, 42); // SCK, MISO, MOSI, CS
        if (SD.begin(42)) {
            sd_initialized = true;
        }
    }

    if (sd_initialized) {
        uint8_t cardType = SD.cardType();
        if (cardType == CARD_NONE) {
            const char* items[] = {"No SD card attached", "Press O to return"};
            gui.drawListMenu("SD Info", items, 2, 1);
        } else {
            uint64_t cardSize = SD.cardSize() / (1024 * 1024);
            uint64_t usedSpace = SD.usedBytes() / (1024 * 1024);
            char sizeBuf[32];
            char usedBuf[32];
            snprintf(sizeBuf, sizeof(sizeBuf), "Size: %llu MB", cardSize);
            snprintf(usedBuf, sizeof(usedBuf), "Used: %llu MB", usedSpace);
            const char* items[] = {sizeBuf, usedBuf, "Press O to return"};
            gui.drawListMenu("SD Info", items, 3, 2);
        }
    } else {
        const char* items[] = {"SD Mount Failed", "Press O to return"};
        gui.drawListMenu("SD Info", items, 2, 1);
    }

    // Wait for 'O' to return
    while (true) {
        char key = keypad.getKey();
        if (key == 'O') {
            break;
        } else if (key == 'E') {
            exit_settings = true;
            break;
        }
        delay(50);
    }
}

static void on_contrast_left() {
    if (current_contrast > 10) current_contrast -= 10;
    gui.setContrast(current_contrast);
    prefs.putUChar("contrast", current_contrast);
}

static void on_contrast_right() {
    if (current_contrast < 245) current_contrast += 10;
    gui.setContrast(current_contrast);
    prefs.putUChar("contrast", current_contrast);
}

/*
static void on_bt_select() {
    const char* items[] = {"Toggle ON/OFF", "Scan Devices", "Show Paired", "Back"};
    uint8_t sel = 0;
    bool exit_menu = false;
    while (!exit_menu) {
        gui.drawListMenu("Bluetooth", items, 4, sel);
        char key = keypad.getKey();
        if (key) {
            if (key == 'E') break;
            else if (key == 'D') sel = (sel + 1) % 4;
            else if (key == 'U') sel = (sel == 0) ? 3 : sel - 1;
            else if (key == 'O') {
                if (sel == 0) {
                    bluetooth_enabled = !bluetooth_enabled;
                    prefs.putBool("bt_en", bluetooth_enabled);
                    if (!bluetooth_enabled) {
                        a2dp_source.end();
                    }
                } else if (sel == 1) {
                    // Scan for devices and let user pick
                    bt_scan_results.clear();
                    bt_scanning = true;
                    bt_scan_start_ms = millis();
                    a2dp_source.end();
                    a2dp_source.set_ssid_callback(bt_scan_cb);
                    a2dp_source.start();

                    uint8_t scan_sel = 0;
                    bool scan_exit = false;
                    while (!scan_exit) {
                        std::vector<const char*> citems;
                        for (auto &n : bt_scan_results) citems.push_back(n.c_str());
                        if (citems.empty()) {
                            const char* msg[] = {"Scanning...", "Press O to cancel"};
                            gui.drawListMenu("Scan", msg, 2, 0);
                        } else {
                            gui.drawListMenu("Scan", citems.data(), citems.size(), scan_sel);
                        }

                        char k = keypad.getKey();
                        if (k) {
                            if (k == 'E') break;
                            else if (k == 'D' && !citems.empty()) scan_sel = (scan_sel + 1) % citems.size();
                            else if (k == 'U' && !citems.empty()) scan_sel = (scan_sel == 0) ? citems.size() - 1 : scan_sel - 1;
                            else if (k == 'O') {
                                if (citems.empty()) { scan_exit = true; }
                                else {
                                    String chosen = bt_scan_results[scan_sel];
                                    prefs.putString("bt_name", chosen);
                                    bt_last_name = chosen;
                                    bluetooth_enabled = true;
                                    prefs.putBool("bt_en", true);
                                    // connect to chosen device
                                    a2dp_source.end();
                                    a2dp_source.set_ssid_callback(nullptr);
                                    a2dp_source.start(chosen.c_str());
                                    scan_exit = true;
                                }
                            }
                            delay(150);
                        }

                        // stop scan after 10s
                        if (millis() - bt_scan_start_ms > 10000) {
                            a2dp_source.cancel_discovery();
                        }
                        delay(50);
                    }
                    bt_scanning = false;
                    a2dp_source.cancel_discovery();
                } else if (sel == 2) {
                    const char* status = bluetooth_enabled ? "BT ON" : "BT OFF";
                    char namebuf[48];
                    if (bt_last_name.length() > 0) snprintf(namebuf, sizeof(namebuf), "Paired: %s", bt_last_name.c_str());
                    else snprintf(namebuf, sizeof(namebuf), "No paired device");
                    const char* msg[] = {status, namebuf, "Press O to return"};
                    gui.drawListMenu("Paired", msg, 3, 2);
                    while (true) {
                        char k = keypad.getKey();
                        if (k == 'O' || k == 'E') break;
                        delay(50);
                    }
                } else {
                    exit_menu = true;
                }
            }
            delay(150);
        }
        delay(50);
    }
}
*/
static void on_back_select() {
    exit_settings = true;
}

/*
static bool bt_scan_cb(const char* name, esp_bd_addr_t address, int rssi) {
    (void)address;
    (void)rssi;
    if (!bt_scanning || !name) return false;
    String dev = String(name);
    for (auto &n : bt_scan_results) {
        if (n == dev) return false;
    }
    bt_scan_results.push_back(dev);
    return false; // keep scanning
}
*/

// --- Main Functions ---

void settings_init() {
    prefs.begin("phone_settings", false);
    current_contrast = prefs.getUChar("contrast", 200);
    bluetooth_enabled = prefs.getBool("bt_en", false);
    bt_last_name = prefs.getString("bt_name", "");
    
    // Apply initial contrast
    gui.setContrast(current_contrast);
}

bool settings_get_bt() {
    return bluetooth_enabled;
}

void settings_set_bt_enabled(bool en) {
    bluetooth_enabled = en;
    prefs.putBool("bt_en", bluetooth_enabled);
}

String settings_get_bt_name() {
    return bt_last_name;
}

void settings_set_bt_name(const String &name) {
    bt_last_name = name;
    prefs.putString("bt_name", bt_last_name);
}

void settings_run() {
    exit_settings = false;
    selected_setting = 0;
    
    const char* display_items[NUM_SETTINGS];
    char buffers[NUM_SETTINGS][32];

    while (!exit_settings) {
        // Prepare display strings
        for (uint8_t i = 0; i < NUM_SETTINGS; i++) {
            if (settings[i].onDraw) {
                settings[i].onDraw(buffers[i], sizeof(buffers[i]));
                display_items[i] = buffers[i];
            } else {
                display_items[i] = settings[i].name;
            }
        }

        gui.drawListMenu("SETTINGS", display_items, NUM_SETTINGS, selected_setting);

        char key = keypad.getKey();
        if (key) {
            if (key == 'E') {
                exit_settings = true;
            } else if (key == 'D') {
                selected_setting = (selected_setting + 1) % NUM_SETTINGS;
            } else if (key == 'U') {
                if (selected_setting == 0) selected_setting = NUM_SETTINGS - 1;
                else selected_setting--;
            } else if (key == 'L') {
                if (settings[selected_setting].onLeft) settings[selected_setting].onLeft();
            } else if (key == 'R') {
                if (settings[selected_setting].onRight) settings[selected_setting].onRight();
            } else if (key == 'O') {
                if (settings[selected_setting].onSelect) settings[selected_setting].onSelect();
            }
            delay(150); // Debounce
        }
        delay(50);
    }
}
