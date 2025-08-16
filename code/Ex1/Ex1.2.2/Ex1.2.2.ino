#include <OneButton.h>

int pins[3] = { 9, 10, 11 };
int buttonPin = 2;
OneButton button(buttonPin, true, true);

bool isOn = false;
int color = 0;
int brightness = 0;
int step = 5;

void setup() {
  for (int i = 0; i < 3; i++) pinMode(pins[i], OUTPUT);
  button.attachLongPressStart(switchPower);
}

void loop() {
  button.tick();

  if (isOn) {
    for (int i = 0; i < 3; i++) {
      analogWrite(pins[i], i == color ? brightness : 0);
    }

    brightness += step;
    if (brightness >= 255 || brightness <= 0) {
      step = -step;
      if (brightness <= 0) color = (color + 1) % 3;
    }

    delay(20);
  } else {
    for (int i = 0; i < 3; i++) analogWrite(pins[i], 0);
    delay(10);
  }
}

void switchPower() {
  isOn = !isOn;
}
