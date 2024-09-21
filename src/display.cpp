#include "config.h"

#include <Wire.h>
// #include <SPI.h>
#include <U8g2lib.h>
#include <cmath>

U8G2_SSD1306_128X64_NONAME_1_SW_I2C
u8g2(U8G2_R0, 4, 5, U8X8_PIN_NONE);

void Display::setup() { u8g2.begin(); }

void Display::show(int val, int maxVal) {
  char buff[20];

  sprintf(buff, "%d", val);
  u8g2.firstPage();
  do {
    u8g2.drawFrame(0, 10, 128, 20);
    u8g2.drawBox(10, 15, min(110, 110 * val / maxVal), 10);
    u8g2.setFont(u8g2_font_ncenB14_tr);
    int w = u8g2.getStrWidth(buff);
    u8g2.drawStr(64 - (w / 2), 60, buff);
  } while (u8g2.nextPage());
}

void Display::show(const char *buff) {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB14_tr);
    int w = u8g2.getStrWidth(buff);
    u8g2.drawStr(64 - (w / 2), 40, buff);
  } while (u8g2.nextPage());
}