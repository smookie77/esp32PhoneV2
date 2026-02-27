#include "phone_gui.h"

PhoneGUI::PhoneGUI() : u8g2(U8G2_R2) {
    // Constructor initializes the U8g2 object
}

void PhoneGUI::begin() {
    u8g2.setI2CAddress(0x3F << 1); // Or 0x3F * 2
    u8g2.begin();
    u8g2.setPowerSave(0);
    u8g2.setContrast(200);
    u8g2.clearBuffer();
    u8g2.sendBuffer();
}

void PhoneGUI::setContrast(uint8_t contrast) {
    u8g2.setContrast(contrast);
}

void PhoneGUI::clear() {
    u8g2.clearBuffer();
}

void PhoneGUI::send() {
    u8g2.sendBuffer();
}

void PhoneGUI::updateStatusBar(uint8_t battery, uint8_t bluetooth, uint8_t cellular, const char* hourStr) {
    if (hourStr == nullptr) return; 
    
    // Note: In a full implementation, we might want to optimize partial updates
    // For now, we assume this is called as part of a full frame draw or we clear here
    // But since u8g2 is buffered, we usually clear, draw everything, then send.
    
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);
    u8g2.setFont(u8g2_font_6x10_tr); 

    // Battery
    switch(battery){
      case 0: u8g2.drawXBM(102, 1, 26, 8, image_Battery_empty_bits); break; 
      case 1: u8g2.drawXBM(102, 1, 26, 8, image_Battery_low_bits); break;
      case 2: u8g2.drawXBM(102, 1, 26, 8, image_Battery_mid_bits); break;
      case 3: u8g2.drawXBM(102, 1, 26, 8, image_Battery_hi_bits); break;
      case 4: u8g2.drawXBM(102, 1, 26, 8, image_Battery_full_bits); break;
      default: u8g2.drawXBM(102, 1, 26, 8, image_Battery_empty_bits); break;
    }

    // Bluetooth
    if(bluetooth == 1){
      u8g2.drawXBM(74, 1, 5, 8, image_Bluetooth_Idle_bits);
    } else if(bluetooth == 2){
      u8g2.drawXBM(70, 1, 9, 8, image_Bluetooth_Connected_bits);
    }

    // Status bar line
    u8g2.drawLine(127, 10, 0, 10);

    // Cellular
    switch(cellular){
      case 0: u8g2.drawXBM(82, 1, 18, 8, image_Cellular_Strenght_none_bits); break;
      case 1: u8g2.drawXBM(82, 1, 18, 8, image_Cellular_Strenght_1_bits); break;
      case 2: u8g2.drawXBM(82, 1, 18, 8, image_Cellular_Strenght_2_bits); break;
      case 3: u8g2.drawXBM(82, 1, 18, 8, image_Cellular_Strenght_3_bits); break;
      case 4: u8g2.drawXBM(82, 1, 18, 8, image_Cellular_Strenght_4_bits); break;
      case 5: u8g2.drawXBM(82, 1, 18, 8, image_Cellular_Strenght_5_bits); break;
      default: u8g2.drawXBM(82, 1, 18, 8, image_Cellular_Strenght_none_bits); break;
    }

    // Clock
    u8g2.drawStr(1, 9, hourStr); 
}

void PhoneGUI::drawMainMenu(uint8_t battery, uint8_t bluetooth, uint8_t cellular, const char* hourStr, const char* centerName, const unsigned char* centerIcon, const unsigned char* leftIcon, const unsigned char* rightIcon, uint8_t currentApp, uint8_t totalApps) {
    clear();
    updateStatusBar(battery, bluetooth, cellular, hourStr); 

    // Draw Icons based on selection
    u8g2.drawXBM(49, 20, 32, 32, centerIcon); 
    u8g2.drawXBM(13, 27, 16, 16, leftIcon); 
    u8g2.drawXBM(100, 27, 16, 16, rightIcon); 
    
    char buf[32];
    snprintf(buf, sizeof(buf), "> %s", centerName);
    u8g2.drawStr(0, 63, buf);

    // Frames
    u8g2.drawFrame(10, 25, 22, 20); // Left
    u8g2.drawFrame(46, 17, 38, 36); // Middle
    u8g2.drawFrame(97, 25, 22, 20); // Right

    // Application NumberIndicator (Bottom Right)
    char idxBuf[16];
    snprintf(idxBuf, sizeof(idxBuf), "%d/%d", currentApp, totalApps);
    u8g2.drawStr(128 - u8g2.getStrWidth(idxBuf), 63, idxBuf);

    // Arrows
    u8g2.drawXBM(87, 33, 7, 5, image_arrow_right_bits);
    u8g2.drawXBM(35, 33, 7, 5, image_arrow_left_bits);

    send();
}

void PhoneGUI::drawListMenu(const char* title, const char** items, uint8_t numItems, uint8_t selectedIndex) {
    clear();
    
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);
    u8g2.setFont(u8g2_font_6x10_tr); 

    // Draw Title Bar
    u8g2.drawBox(0, 0, 128, 12);
    u8g2.setDrawColor(0);
    u8g2.drawStr(2, 10, title);
    u8g2.setDrawColor(1);

    // Draw List Items
    uint8_t startY = 22;
    uint8_t lineHeight = 12;
    
    // Simple scrolling logic: keep selected item in view
    uint8_t visibleItems = 4;
    uint8_t startIndex = 0;
    if (selectedIndex >= visibleItems) {
        startIndex = selectedIndex - visibleItems + 1;
    }

    for (uint8_t i = 0; i < visibleItems && (startIndex + i) < numItems; i++) {
        uint8_t itemIndex = startIndex + i;
        uint8_t y = startY + (i * lineHeight);
        
        if (itemIndex == selectedIndex) {
            u8g2.drawStr(2, y, ">");
        }
        u8g2.drawStr(12, y, items[itemIndex]);
    }
    
    send();
}
