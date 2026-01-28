#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <bitmaps.h>
#include <services.h>
#include <servicemgr.h>
U8G2_ST7567_ENH_DG128064I_F_HW_I2C u8g2(U8G2_R2);


void scrDraw_statusBar(uint8_t battery, uint8_t bluetooth, uint8_t cellular,  char* hourStr) {
    if (hourStr == nullptr) return; // Null check
    
    u8g2.clearBuffer();
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);
    u8g2.setFont(u8g2_font_6x10_tr); // Set font early

    // Battery
    switch(battery){
      case 0: u8g2.drawXBM(102, 1, 26, 8, image_Battery_empty_bits); break; 
      case 1: u8g2.drawXBM(102, 1, 26, 8, image_Battery_low_bits); break;
      case 2: u8g2.drawXBM(102, 1, 26, 8, image_Battery_mid_bits); break;
      case 3: u8g2.drawXBM(102, 1, 26, 8, image_Battery_hi_bits); break;
      case 4: u8g2.drawXBM(102, 1, 26, 8, image_Battery_full_bits); break;
      default: u8g2.drawXBM(102, 1, 26, 8, image_Battery_empty_bits); break;
    }
    // Bluetooth_Idle
    if(bluetooth == 0){
      __asm__("NOP");
    } else if(bluetooth == 1){
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


    u8g2.sendBuffer();
}

void scrDraw_mainMenu(uint8_t battery, uint8_t bluetooth, uint8_t cellular, uint8_t selectedIcon){
    scrDraw_statusBar(battery, bluetooth, cellular, "22:36");
    // App menu
    selectedIcon = selectedIcon % 3;
    switch(selectedIcon){
      case 0: u8g2.drawXBM(50, 20, 28, 32, image_menu_tools_full_bits); // Middle icon
        u8g2.drawXBM(13, 27, 16, 16, image_message_mail_small_bits); // Left icon
        u8g2.drawXBM(100, 27, 16, 16, image_music_radio_small_bits); // Right icon
        u8g2.drawStr(0, 63, "> SETTINGS");
        break;

        case 1: u8g2.drawXBM(49, 20, 32, 32, image_music_radio_full_bits); // Middle icon
        u8g2.drawXBM(13, 27, 16, 16, image_menu_tools_small_bits); // Left icon
        u8g2.drawXBM(100, 27, 16, 16, image_message_mail_small_bits); // Right icon
        u8g2.drawStr(0, 63, "> MUSIC");
        break;

        case 2: u8g2.drawXBM(49, 20, 32, 32, image_message_mail_full_bits); // Middle icon
        u8g2.drawXBM(13, 27, 16, 16, image_music_radio_small_bits); // Left icon
        u8g2.drawXBM(100, 27, 16, 16, image_menu_tools_small_bits); // Right icon
        u8g2.drawStr(0, 63, "> MESSAGES");
        break;
      
    }


    // Left app frame
    u8g2.drawFrame(10, 25, 22, 20);

    // Middle app frame
    u8g2.drawFrame(46, 17, 38, 36);
    
    // Right app frame
    u8g2.drawFrame(97, 25, 22, 20);

    // arrow_right
    u8g2.drawXBM(87, 33, 7, 5, image_arrow_right_bits);

    // arrow_left
    u8g2.drawXBM(35, 33, 7, 5, image_arrow_left_bits);

}


  service_status_t log_buf;

void setup(){
  Serial.begin(115200);
  
  // POWER through GPIO
  pinMode(2, OUTPUT);
  pinMode(42, OUTPUT);
  digitalWrite(2, HIGH);
  digitalWrite(42, LOW);

  Wire.begin(40,41,1000000);
  u8g2.setI2CAddress(0x3F << 1);
  u8g2.begin();
  u8g2.setPowerSave(0);
  u8g2.setContrast(200);

  u8g2.clearBuffer();


  delay(100);
}

int a,b,c,d=0;
void loop(){
  a++;
  if (a > 4){
    a = 0;
  }
  b++;
  if (b > 2){
    b = 0;
  }
  c++;
  if (c > 5){
    c = 0;
  }
  d++;

  service_start("wifi");
  service_get_status("wifi", &log_buf);
  Serial.println(log_buf.log);

  delay(500);

  service_stop("wifi");
  service_get_status("wifi", &log_buf);
  Serial.println(log_buf.log);

  delay(500);

  scrDraw_mainMenu(a, b, c, d);

  delay(2000);
  

}