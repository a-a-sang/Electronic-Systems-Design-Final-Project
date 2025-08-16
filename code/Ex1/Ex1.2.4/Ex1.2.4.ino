#include <Servo.h>

const int pinA = 2;
const int pinB = 3;
int count = 0;
int lastA = LOW;
Servo servo;

void setup() {
  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);
  servo.attach(9);
  servo.write(0);
}

void loop() {
  int a = digitalRead(pinA);
  if (a != lastA && a == HIGH) {
    int b = digitalRead(pinB);
    if (b != a) count++;
    else count--;
    int angle = count / 12;
    angle = constrain(angle, 0, 180);
    servo.write(angle);
  }
  lastA = a;
}
