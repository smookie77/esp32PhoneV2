#include "music_app.h"
#include "phone_gui.h"
#include "keypad_driver.h"
#include "settings_app.h"
#include <SPI.h>
#include <SD.h>
#include <vector>
#include <string>
#include <cmath>
#include "Audio.h" // Ensure Audio is visible

// Simple menu indices
enum MusicMenuItem {
    // MENU_OUTPUT = 0, // Disabled
    MENU_MP3_SD = 0,
    MENU_RADIO,
    MENU_NOW_PLAYING,
    MENU_TEST_TONE,
    MENU_BACK,
    MENU_COUNT
};

static const char* menu_labels[MENU_COUNT] = {
//    "Audio Output",
    "MP3 from SD",
    "Internet Radio",
    "Now Playing",
    "Test 1kHz Sine",
    "Back"
};

static String current_title = "None";

// Forward declarations
static void ensure_sd();
// static void menu_audio_output(); // Disabled
static void menu_mp3_sd();
static void menu_radio();
static void menu_now_playing();
static void menu_test_tone();

void music_app_run() {
    uint8_t sel = 0;
    bool exit_app = false;

    while (!exit_app) {
        audio.loop(); // Must be called in main loop
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
                    case MENU_NOW_PLAYING: menu_now_playing(); break;
                    case MENU_TEST_TONE: menu_test_tone(); break;
                    case MENU_BACK: exit_app = true; break;
                }
            }
            delay(50); // Reduced delay for smoother loop
        }
        delay(10); // Reduced delay
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
            audio.loop();
            char key = keypad.getKey();
            if (key == 'O' || key == 'E') break;
            delay(50);
        }
        return;
    }

    uint8_t sel = 0;
    bool exit_menu = false;
    while (!exit_menu) {
        audio.loop(); // Maintain playback
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
                current_title = files[sel]; // Store title
                audio.stopSong();
                if (audio.connecttoFS(SD, path.c_str())) {
                    menu_now_playing(); // Go to player immediately
                }
                // When returning from Now Playing, we stay in MP3 menu or exit?
                // Let's stay in MP3 menu so user can pick another song.
                // If they want to go back, they press E.
            }
            delay(10);
        }
        delay(10);
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
            audio.loop();
            char key = keypad.getKey();
            if (key == 'O' || key == 'E') break;
            delay(50);
        }
        return;
    }

    uint8_t sel = 0;
    bool exit_menu = false;
    while (!exit_menu) {
        audio.loop();
        std::vector<const char*> citems;
        for (auto &n : names) citems.push_back(n.c_str());
        gui.drawListMenu("Radio", citems.data(), citems.size(), sel);

        char key = keypad.getKey();
        if (key) {
            if (key == 'E') break;
            else if (key == 'D') sel = (sel + 1) % names.size();
            else if (key == 'U') sel = (sel == 0) ? names.size() - 1 : sel - 1;
            else if (key == 'O') {
                current_title = names[sel];
                audio.stopSong();
                audio.connecttohost(urls[sel].c_str());
                menu_now_playing();
                // exit_menu = true; // Optional: exit on play
            }
            delay(10);
        }
        delay(10);
    }
}

static void menu_now_playing() {
    bool exit_player = false;
    while (!exit_player) {
        audio.loop();

        // Status Line
        String status = audio.isRunning() ? "Playing" : "Paused";
        
        // Time
        uint32_t current = audio.getAudioCurrentTime();
        uint32_t total = audio.getAudioFileDuration();
        char timeBuf[32];
        snprintf(timeBuf, sizeof(timeBuf), "%02lu:%02lu / %02lu:%02lu", current/60, current%60, total/60, total%60);

        const char* items[] = {
            current_title.c_str(),
            status.c_str(), 
            timeBuf,
            "O:Pause E:Back"
        };
        
        // We use toggle logic for selection purely to use drawListMenu, but we ignore selection
        gui.drawListMenu("Now Playing", items, 4, 1);
        
        char key = keypad.getKey();
        if (key == 'E') {
            exit_player = true; 
        } else if (key == 'O') {
            audio.pauseResume();
        }
        delay(20);
    }
}

static void menu_test_tone() {
    const char* items[] = {"Playing 1kHz", "Press O/E to stop"};
    gui.drawListMenu("Test Tone", items, 2, 1);

    // Stop internal audio tasks
    audio.stopSong();

    // Prepare I2S
    // audio.setSampleRate(48000); // Private method, configuring manually below
    audio.setVolume(21); // Max volume for test

    i2s_chan_handle_t tx_handle = audio.getI2sTxHandle();
    if (!tx_handle) return;
    
    // Configure I2S Clock manually (since setSampleRate is private)
    i2s_std_clk_config_t clk_cfg = {};
    clk_cfg.sample_rate_hz = 48000;
    clk_cfg.clk_src = I2S_CLK_SRC_DEFAULT;
    clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_256;

    i2s_channel_disable(tx_handle);
    i2s_channel_reconfig_std_clock(tx_handle, &clk_cfg);
    i2s_channel_enable(tx_handle);

    // Pre-calculate 1kHz sine wave (48kHz sample rate -> 48 samples/cycle)
    const int sample_rate = 48000;
    const int freq = 1000;
    const int samples_per_cycle = sample_rate / freq; // 48
    const size_t cycles = 20; // Enough data to minimize overhead
    const size_t buf_samples = samples_per_cycle * cycles; 
    std::vector<int16_t> tone_buf(buf_samples * 2);

    const int16_t amplitude = 12000; // ~40% full scale
    const float PI_F = 3.14159265f;
    for (size_t i = 0; i < buf_samples; i++) {
        float angle = (2.0f * PI_F * i) / samples_per_cycle;
        int16_t sample = (int16_t)(sin(angle) * amplitude);
        tone_buf[i * 2] = sample;       // Left
        tone_buf[i * 2 + 1] = sample;   // Right
    }

    size_t bytes_written = 0;
    bool stop = false;
    while (!stop) {
        // Write audio data (blocking)
        i2s_channel_write(tx_handle, tone_buf.data(), tone_buf.size() * sizeof(int16_t), &bytes_written, 1000);
        
        char key = keypad.getKey();
        if (key == 'O' || key == 'E') stop = true;
        delay(10);
    }

    // Cleanup
    audio.stopSong(); // Ensures clean state for next operations
}
