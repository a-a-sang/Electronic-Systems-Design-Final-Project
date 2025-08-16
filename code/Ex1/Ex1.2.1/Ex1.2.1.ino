int pins[3] = { 9, 10, 11 };
int color = 0;  //当前颜色索引（0=红, 1=绿, 2=蓝）
int brightness = 0;
int step = 5;

void setup() {
  for (int i = 0; i < 3; i++) {
    pinMode(pins[i], OUTPUT);
  }
}

void loop() {

  for (int i = 0; i < 3; i++) {
    analogWrite(pins[i], i == color ? brightness : 0);  //只有一种颜色
  }

  brightness += step;
  if (brightness >= 255 || brightness <= 0) {
    step = -step;  //反向变化
    if (brightness <= 0) {
      color = (color + 1) % 3;  //切换到下一种颜色
    }
  }

  delay(20);
}
