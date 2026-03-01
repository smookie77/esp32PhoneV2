#include "music_app.h"
#include "phone_gui.h"
#include "keypad_driver.h"
#include "settings_app.h"
#include <SPI.h>
#include <SD.h>
#include <vector>
#include <string>
#include <cmath>

// Simple menu indices
enum MusicMenuItem {
    // MENU_OUTPUT = 0, // Disabled
    MENU_MP3_SD = 0,
    MENU_RADIO,
    MENU_TEST_TONE,
    MENU_BACK,
    MENU_COUNT
};

static const char* menu_labels[MENU_COUNT] = {
//    "Audio Output",
    "MP3 from SD",
    "Internet Radio",
    "Test 1kHz Sine",
    "Back"
};

// Forward declarations
static void ensure_sd();
// static void menu_audio_output(); // Disabled
static void menu_mp3_sd();
static void menu_radio();
static void menu_test_tone();

void music_app_run() {
    uint8_t sel = 0;
    bool exit_app = false;

    while (!exit_app) {
        gui.drawListMenu("MUSIC", menu_labels, MENU_COUNT, sel);
        char key = keypad.getKey();
        if (key) {
            if (key == 'E') {
                break;
            } else if (key == 'D') {
                sel = (sel + 1) % MENU_COUNT;
            } else if (key == 'U') {
                sel = (sel == 0) ? MENU_COUNT - 1 : sel - 1;
            } else if (key == 'O') {
                switch (sel) {
                    // case MENU_OUTPUT: menu_audio_output(); break;
                    case MENU_MP3_SD: menu_mp3_sd(); break;
                    case MENU_RADIO: menu_radio(); break;
                    case MENU_TEST_TONE: menu_test_tone(); break;
                    case MENU_BACK: exit_app = true; break;
                }
            }
            delay(150);
        }
        delay(50);
    }
}

// --- Helpers ---
static void ensure_sd() {
    static bool sd_init = false;
    if (!sd_init) {
        SPI.begin(2, 44, 43, 42); // match settings_app
        sd_init = SD.begin(42);
    }
}

/*
static void menu_audio_output() {
    const char* opts[] = {"Internal I2S", "Bluetooth"};
    uint8_t sel = (current_route == ROUTE_BT) ? 1 : 0;
    bool exit_menu = false;
    while (!exit_menu) {
        gui.drawListMenu("Audio Output", opts, 2, sel);
        char key = keypad.getKey();
        if (key) {
            if (key == 'E') break;
            else if (key == 'D' || key == 'U') sel ^= 1;
            else if (key == 'O') {
                current_route = (sel == 1) ? ROUTE_BT : ROUTE_I2S;
                if (current_route == ROUTE_BT) {
                    settings_set_bt_enabled(true);
                    a2dp_source.end();
                    a2dp_source.set_data_callback(get_bluetooth_data);
                    String target = settings_get_bt_name();
                    if (target.length() > 0) a2dp_source.start(target.c_str());
                    else a2dp_source.start();
                } else {
                    settings_set_bt_enabled(false);
                    a2dp_source.end();
                }
                exit_menu = true;
            }
            delay(150);
        }
        delay(50);
    }
}
*/

static void menu_mp3_sd() {
    ensure_sd();
    std::vector<String> files;
    File root = SD.open("/");
    if (root) {
        File entry;
        while ((entry = root.openNextFile())) {
            if (!entry.isDirectory()) {
                String name = entry.name();
                name.toLowerCase();
                if (name.endsWith(".mp3")) {
                    files.push_back(String(entry.name()));
                }
            }
            entry.close();
        }
        root.close();
    }

    if (files.empty()) {
        const char* items[] = {"No MP3 found", "Press O to return"};
        gui.drawListMenu("MP3", items, 2, 1);
        while (true) {
            char key = keypad.getKey();
            if (key == 'O' || key == 'E') break;
            delay(50);
        }
        return;
    }

    uint8_t sel = 0;
    bool exit_menu = false;
    while (!exit_menu) {
        // Build C-array of pointers for drawListMenu
        std::vector<const char*> citems;
        for (auto &f : files) citems.push_back(f.c_str());
        gui.drawListMenu("MP3", citems.data(), citems.size(), sel);

        char key = keypad.getKey();
        if (key) {
            if (key == 'E') break;
            else if (key == 'D') sel = (sel + 1) % files.size();
            else if (key == 'U') sel = (sel == 0) ? files.size() - 1 : sel - 1;
            else if (key == 'O') {
                String path = String("/") + files[sel];
                audio.stopSong();
                if (audio.connecttoFS(SD, path.c_str())) {
                    // playback proceeds inside audio loop task
                }
                exit_menu = true;
            }
            delay(150);
        }
        delay(50);
    }
}

static void menu_radio() {
    ensure_sd();
    File f = SD.open("/stations.txt", FILE_READ);
    std::vector<String> names;
    std::vector<String> urls;
    if (f) {
        while (f.available()) {
            String line = f.readStringUntil('\n');
            line.trim();
            if (line.length() == 0) continue;
            int comma = line.indexOf(',');
            if (comma > 0) {
                names.push_back(line.substring(0, comma));
                urls.push_back(line.substring(comma + 1));
            }
        }
        f.close();
    }
    if (names.empty()) {
        const char* items[] = {"stations.txt missing", "or empty", "Press O to return"};
        gui.drawListMenu("Radio", items, 3, 2);
        while (true) {
            char key = keypad.getKey();
            if (key == 'O' || key == 'E') break;
            delay(50);
        }
        return;
    }

    uint8_t sel = 0;
    bool exit_menu = false;
    while (!exit_menu) {
        std::vector<const char*> citems;
        for (auto &n : names) citems.push_back(n.c_str());
        gui.drawListMenu("Radio", citems.data(), citems.size(), sel);

        char key = keypad.getKey();
        if (key) {
            if (key == 'E') break;
            else if (key == 'D') sel = (sel + 1) % names.size();
            else if (key == 'U') sel = (sel == 0) ? names.size() - 1 : sel - 1;
            else if (key == 'O') {
                audio.stopSong();
                audio.connecttohost(urls[sel].c_str());
                exit_menu = true;
            }
            delay(150);
        }
        delay(50);
    }
}

static void menu_test_tone() {
    // Simple 1kHz sine stream for a short duration to verify output
    const char* items[] = {"Playing 1kHz", "Press O to stop"};
    gui.drawListMenu("Test Tone", items, 2, 1);

    const float freq = 1000.0f;
    const int sample_rate = 48000;
    const int frames = 256;
    const float PI_F = 3.14159265f;
    static int16_t tone_buf[frames * 2];
    uint32_t phase = 0;
    const uint32_t phase_inc = (uint32_t)((freq / sample_rate) * (1ull << 32));

    bool stop = false;
    while (!stop) {
        for (int i = 0; i < frames; i++) {
            phase += phase_inc;
            float s = sinf((float)phase * 2 * PI_F / (float)(1ull << 32));
            int16_t v = (int16_t)(s * 16000);
            tone_buf[i * 2] = v;
            tone_buf[i * 2 + 1] = v;
        }

        if (false /* current_route == ROUTE_BT && audio_buffer */) {
            // Disabled
        } else {
            // direct I2S using audio library helper
            audio.stopSong();
            // Write via raw I2S is non-trivial without handle; keep placeholder
        }

        char key = keypad.getKey();
        if (key == 'O' || key == 'E') stop = true;
        delay(10);
    }
}
