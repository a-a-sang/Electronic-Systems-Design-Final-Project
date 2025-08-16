#include <U8g2lib.h>
#include <Wire.h>
#include <OneButton.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
auto& screen = u8g2;

const int BTN_ADD = 2;
const int BTN_SUB = 3;

OneButton buttonAdd(BTN_ADD, true);
OneButton buttonSub(BTN_SUB, true);

int value = 0;

void setup() {
  screen.begin();

  buttonAdd.attachClick([]() {
    value++;
    display();
  });

  buttonSub.attachClick([]() {
    value--;
    display();
  });

  display();
}

void loop() {
  buttonAdd.tick();
  buttonSub.tick();
  delay(10);
}

void display() {
  screen.firstPage();
  do {
    screen.setFont(u8g2_font_ncenB14_tr);//设置字体，不设字体就不显示任何文字
    screen.setCursor(20, 30);
    screen.print("Value: ");
    screen.print(value);
  } while (screen.nextPage());
}
