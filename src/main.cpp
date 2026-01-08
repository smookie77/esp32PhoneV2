#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <bitmaps.h>
#include <services.h>
#include <servicemgr.h>
U8G2_ST7567_ENH_DG128064I_F_HW_I2C u8g2(U8G2_R2);


void draw(void) {
    u8g2.clearBuffer();
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);
    // Battery
    u8g2.drawXBM(102, 1, 26, 8, image_Battery_bits); //EXPORT

    // Bluetooth_Idle
    u8g2.drawXBM(74, 1, 5, 8, image_Bluetooth_Idle_bits); //EXPORT

    // Layer 3
    u8g2.drawLine(127, 10, 0, 10);

    // Layer 4
    u8g2.drawXBM(82, 1, 18, 8, image_Layer_4_bits); //EXPORT

    // Layer 5
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(1, 9, "11:11"); //EXPORT

    // menu_tools
    u8g2.drawXBM(50, 20, 28, 32, image_menu_tools_bits); //EXPORT

    // message_mail
    u8g2.drawXBM(10, 27, 17, 16, image_message_mail_bits); //EXPORT

    // Layer 8
    u8g2.drawFrame(46, 17, 37, 36);

    // Layer 9
    u8g2.drawFrame(8, 25, 21, 20);

    // music_radio
    u8g2.drawXBM(101, 27, 16, 16, image_music_radio_bits); //EXPORT

    // Layer 11
    u8g2.drawFrame(98, 25, 22, 20);

    // arrow_right
    u8g2.drawXBM(87, 33, 7, 5, image_arrow_right_bits);

    // arrow_left
    u8g2.drawXBM(33, 33, 7, 5, image_arrow_left_bits);

    // Layer 14
    u8g2.drawStr(40, 63, "SETTINGS"); //EXPORT

    u8g2.sendBuffer();
}

  service_status_t log_buf;

void setup(){
  Serial.begin(115200);
  Wire.begin(10, 11, 400000);
  u8g2.setI2CAddress(0x3F << 1);
  u8g2.begin();
  u8g2.setPowerSave(0);
  u8g2.setContrast(200);

  u8g2.clearBuffer();

  draw();

  delay(100);
}

void loop(){
  service_start("wifi");
  service_get_status("wifi", &log_buf);
  Serial.println(log_buf.log);

  delay(500);

  service_stop("wifi");
  service_get_status("wifi", &log_buf);
  Serial.println(log_buf.log);

  delay(500);

}