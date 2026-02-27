#ifndef PHONE_GUI_H
#define PHONE_GUI_H

#include <Arduino.h>
#include <U8g2lib.h>
#include <bitmaps.h>

class PhoneGUI {
public:
    PhoneGUI();
    void begin();
    void updateStatusBar(uint8_t battery, uint8_t bluetooth, uint8_t cellular, const char* hourStr);
    void drawMainMenu(uint8_t battery, uint8_t bluetooth, uint8_t cellular, const char* hourStr, const char* centerName, const unsigned char* centerIcon, const unsigned char* leftIcon, const unsigned char* rightIcon, uint8_t currentApp, uint8_t totalApps);
    void drawListMenu(const char* title, const char** items, uint8_t numItems, uint8_t selectedIndex);
    void setContrast(uint8_t contrast);
    void clear();
    void send();

private:
    U8G2_ST7567_ENH_DG128064I_F_HW_I2C u8g2;
};

extern PhoneGUI gui;

#endif
