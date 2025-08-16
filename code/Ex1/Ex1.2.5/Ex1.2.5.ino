#include <U8g2lib.h>
#include <Wire.h>
#include <Encoder.h>

const int pinA = 2;
const int pinB = 3;
long count = 0;
int lastA = LOW;
const int perRev = 360;              // 一圈脉冲
const long full = (long)perRev * 4;  //一圈总计数
const int x = 10, y = 30, w = 108, h = 12;

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
Encoder enc(pinA, pinB);

void setup() {
  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
}

void loop() {
  count = enc.read();
  if (count < 0) count = 0;
  if (count > full) count = full;
  long angle = (count * 360) / full;
  int fill = (int)(angle * w / 360);  //转一圈进度条走完
  u8g2.clearBuffer();
  u8g2.setCursor(x, y - 6);
  u8g2.print("Angle: ");
  u8g2.print(angle);
  u8g2.drawBox(x, y, fill, h);
  u8g2.sendBuffer();
  delay(15);
}
