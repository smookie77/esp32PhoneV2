#include "settings_app.h"
#include "phone_gui.h"
#include "keypad_driver.h"
#include <Preferences.h>
#include <WiFi.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>

// Global state for settings
static Preferences prefs;
static uint8_t current_contrast = 200;
static bool bluetooth_enabled = false;
static bool sd_initialized = false;

// Forward declarations for setting callbacks
static void on_wifi_select();
static void on_sd_select();
static void on_contrast_left();
static void on_contrast_right();
static void on_bt_left();
static void on_bt_right();
static void on_back_select();

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

static void draw_bt(char* buffer, size_t len) {
    snprintf(buffer, len, "Bluetooth: %s", bluetooth_enabled ? "ON" : "OFF");
}

// The list of settings
static SettingItem settings[] = {
    {"WiFi Info", on_wifi_select, nullptr, nullptr, nullptr},
    {"SD Card Info", on_sd_select, nullptr, nullptr, nullptr},
    {"Contrast", nullptr, on_contrast_left, on_contrast_right, draw_contrast},
    {"Bluetooth", nullptr, on_bt_left, on_bt_right, draw_bt},
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

static void on_bt_left() {
    bluetooth_enabled = !bluetooth_enabled;
    prefs.putBool("bt_en", bluetooth_enabled);
}

static void on_bt_right() {
    on_bt_left(); // Toggle is the same for left/right
}

static void on_back_select() {
    exit_settings = true;
}

// --- Main Functions ---

void settings_init() {
    prefs.begin("phone_settings", false);
    current_contrast = prefs.getUChar("contrast", 200);
    bluetooth_enabled = prefs.getBool("bt_en", false);
    
    // Apply initial contrast
    gui.setContrast(current_contrast);
}

bool settings_get_bt() {
    return bluetooth_enabled;
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
