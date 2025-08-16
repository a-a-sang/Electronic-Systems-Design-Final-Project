#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <NewPing.h>
#include <OneButton.h>
#include <Servo.h>
#include "Bitmaps.h"  // 3D菜单图标与位置
#include "Songs.h"    // 歌曲标题与乐谱

//Mega2560引脚配置
#define R_PIN 5
#define G_PIN 6
#define B_PIN 7
#define BTN1 12
#define BTN2 13
#define ENC_A 2
#define ENC_B 3
#define TRIG 4
#define ECHO A8
#define BUZZ 8
#define SERVO 9

#define MAX_DIS 100  //超声波最大测距（cm）

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

NewPing sonar(TRIG, ECHO, MAX_DIS);

OneButton btn1(BTN1, true);
OneButton btn2(BTN2, true);

Servo sg;

enum State {
  Animation_state,
  Menu_state,
  Flashlight_state,
  Voltage_Meter_state,
  Oscillocope_state,
  Rangefinder_state,
  Music_Player_state
};  //状态名

State state = Animation_state;

const int NUM_MENU_ITEMS = 5;
static int selMenu = 0;
const char* menuItems[] = {
  "Flashlight",
  "Voltage Meter",
  "Oscilloscope",
  "Rangefinder",
  "Music Player"
};
int holdItem = 0;  //当前菜单

volatile int encCnt = 0;  //编码器计数

int ledLvl = -1;  //手电筒挡位（-1 代表关闭）
unsigned long ledOffAt = 0;
bool ledBeatOn = false;  //音乐节拍点亮，超时熄灭

bool th_mode = false;  //是否正在设置阈值
char th_str[10] = "15";
int thr = 15;  //阈值(cm)

//示波器
const byte OSC_X0 = 10;           //波形左边界
const byte OSC_W = 128 - OSC_X0;  //宽
const byte OSC_H = 64;            //高
const byte ainPins[] = { A0, A1, A2, A3 };
const int chCnt = sizeof(ainPins) / sizeof(ainPins[0]);
int chIdx = 0;        //当前模拟通道
byte waveBuf[OSC_W];  //保存过去 OSC_W 个采样点（y 像素）
unsigned long lastSamp = 0;
const unsigned long SMP_INT_MS = 50;  //采样间隔（毫秒）

//音乐播放器控制
unsigned long songDur[NUM_SONGS];  //总时长，setup计算
unsigned long noteEndAt = 0;       //当前音符结束时间
int noteIdx = 0;                   //音符索引
unsigned long songBeg = 0;         //歌曲开始播放时间（进度条显示）
unsigned long pausAt = 0;          //暂停时间
bool isPause = false;              //暂停状态
int songIdx = 0;                   //当前歌曲索引

//编码器
void encodercount() {
  static unsigned long lastMic = 0;  // 上次触发
  unsigned long nowMic = micros();   // 当前
  if (nowMic - lastMic < 1000) return;
  lastMic = nowMic;
  if (digitalRead(ENC_A) == digitalRead(ENC_B)) { encCnt++; }  // 顺时针
  else {
    encCnt--;
  }  //逆时针
}

// 关闭所有 LED
void turnAllLedsOff() {
  analogWrite(R_PIN, 0);
  analogWrite(G_PIN, 0);
  analogWrite(B_PIN, 0);
}

//重置播放
static inline void resetMusicState() {
  isPause = false;
  noteIdx = 0;
  noteEndAt = 0;
  songBeg = 0;
  pausAt = 0;
  ledBeatOn = false;
}

//重置蜂鸣器
static inline void allQuiet() {
  noTone(BUZZ);
  turnAllLedsOff();
}

//开机动画 Ex2.1欢迎语句，Ex3.6开机动画：小火车驶入
void Animation() {
  static int x = -48;             // 小车的起始坐标
  static unsigned long tick = 0;  // 定时器
  if (millis() - tick < 40) return;
  tick = millis();
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(40, 14, "Welcome!");
    int y = 36;
    //地面
    u8g2.drawLine(8, 46, 120, 46);
    // 车身
    u8g2.drawBox(x, y, 14, 8);
    u8g2.drawBox(x + 16, y, 12, 8);
    u8g2.drawBox(x + 32, y, 12, 8);
    // 车轮
    u8g2.drawDisc(x + 3, y + 10, 2);
    u8g2.drawDisc(x + 11, y + 10, 2);
    u8g2.drawDisc(x + 19, y + 10, 2);
    u8g2.drawDisc(x + 27, y + 10, 2);
    u8g2.drawDisc(x + 35, y + 10, 2);
    u8g2.drawDisc(x + 43, y + 10, 2);
  } while (u8g2.nextPage());
  x += 3;
  if (x > 128) x = -48;  //循环播放
}

//菜单 Ex2.2菜单翻动，选定 Ex3.1 3D界面菜单
void Menu() {
  static uint8_t fAnim = 0;                         //当前帧（0~149）
  static int prev = encCnt;                         //上一轮编码器计数
  int diff = encCnt - prev;                         // 变化量
  if (diff > 2) fAnim = (fAnim + 1) % 150;          // 顺时针滚动
  else if (diff < -2) fAnim = (fAnim + 149) % 150;  // 逆时针滚动
  prev = encCnt;
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tr);
    for (int i = 0; i < NUM_MENU_ITEMS; i++) {
      //每个菜单项隔30帧
      int f = (fAnim + 30 * i) % 150;
      byte sfrm;        //图标帧号（从 Bitmap 表取不同缩放/动作）
      byte isz;         //图标尺寸
      bool hl = false;  //是否高亮（当前）
      //进入高亮区
      if (f <= 15) {
        sfrm = f / 2;
        isz = 32;
        hl = true;
      }
      //即将离开高亮区
      else if (f >= 135) {
        sfrm = (150 - f) / 2;
        isz = 32;
        hl = true;
      }
      //小图标
      else {
        sfrm = 10;
        isz = 16;
      }
      //该帧对应图标位置
      byte xi = menu_positions[f][0];
      byte yi = menu_positions[f][1];
      int idx = sfrm + ((selMenu + i) % NUM_MENU_ITEMS) * 11;  //具体位图
      u8g2.drawXBMP(xi - isz / 2, yi - isz / 2, isz, isz, (const unsigned char*)bitmap_icons_array[idx]);
      //显示当前功能名并记录
      if (hl) {
        u8g2.drawStr(40, 60, menuItems[i]);
        holdItem = i;
      }
    }
  } while (u8g2.nextPage());
}

//Ex2.3 手电筒功能
void Flashlight() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_7x14_tr);
    u8g2.drawStr(35, 10, "Flashlight");
  } while (u8g2.nextPage());
  if (ledLvl < 0) {
    analogWrite(R_PIN, 0);
    analogWrite(G_PIN, 0);
    analogWrite(B_PIN, 0);
  }
  //微微蓝色
  else if (ledLvl % 3 == 0) {
    analogWrite(B_PIN, 50);
    analogWrite(G_PIN, 0);
    analogWrite(R_PIN, 0);
  }
  //白色微亮
  else if (ledLvl % 3 == 1) {
    analogWrite(B_PIN, 50);
    analogWrite(G_PIN, 50);
    analogWrite(R_PIN, 50);
  }
  //白色最亮
  else {
    analogWrite(B_PIN, 255);
    analogWrite(G_PIN, 255);
    analogWrite(R_PIN, 255);
  }
}

//电压表 Ex2.4 显示电压，舵机指针 Ex3.6 屏幕指针
void Voltage_Meter() {
  static unsigned long lastDraw = 0;
  unsigned long now = millis();
  if (now - lastDraw < 50) return;  //防止刷新过快
  lastDraw = now;
  int s = analogRead(A0);
  float v = s * 5.0f / 1023.0f;     //电压值 0~5V
  float ang = 10.0f + (v * 32.0f);  //映射到 10°~170°
  if (ang < 10.0f) ang = 10.0f;
  if (ang > 170.0f) ang = 170.0f;
  sg.write((int)ang);
  char vstr[10] = "";
  dtostrf(v, 4, 2, vstr);  //保留两位小数
  const int cx = 90;       //中心 X
  const int cy = 45;       //中心 Y
  const int pinLen = 25;   //指针长度
  //极坐标变换
  float rad = ang * 0.0174532925f;
  int xp = cx + (int)(pinLen * cos(rad));
  int yp = cy - (int)(pinLen * sin(rad));  //y轴向下
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_7x14_tr);
    u8g2.drawStr(10, 12, "Voltage");
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(12, 28, vstr);
    u8g2.drawStr(42, 28, "V");
    u8g2.drawLine(cx, cy, xp, yp);
    u8g2.drawDisc(cx, cy, 2);
    btn1.tick();
    btn2.tick();
  } while (u8g2.nextPage());
}

//测距 Ex2.6 测距警报，调整阈值
//蓝色呼吸灯
void breathingLED() {
  static int val = 0, dir = 1;
  static unsigned long t1 = 0;
  unsigned long t2 = millis();
  if (t2 - t1 < 10) return;
  t1 = t2;
  val += dir * 20;
  if (val > 255) {
    val = 255;
    dir = -1;
  }
  if (val < 0) {
    val = 0;
    dir = 1;
  }
  analogWrite(B_PIN, val);
  analogWrite(G_PIN, 0);
  analogWrite(R_PIN, 0);
}
//大于阈值
void largerthan(float d) {
  // <100cm 显示绿灯（有效测距）
  if (d <= 100.0) {
    analogWrite(G_PIN, 255);
    analogWrite(B_PIN, 0);
    analogWrite(R_PIN, 0);
    noTone(BUZZ);
  }
  // >=100cm 视作超出可视范围：全灭
  else {
    turnAllLedsOff();
    noTone(BUZZ);
  }
}
//小于阈值
void smallerthan(float d) {
  static bool on = false;
  static unsigned long last = 0;
  analogWrite(G_PIN, 0);
  unsigned long now = millis();
  unsigned long cycle = constrain(d * 10 + 100, 50, 1000);  // 闪烁总周期
  unsigned long half = min(cycle, 50UL);                    // ON 时间上限 50ms
  if (now - last >= (on ? half : cycle - half)) {
    on = !on;
    last = now;
    //测距警报
    if (on) {
      tone(BUZZ, 1000);
      analogWrite(R_PIN, 255);
    }
    //警报解除
    else {
      noTone(BUZZ);
      analogWrite(R_PIN, 0);
    }
  }
}

//Ex2.5 测距
void Rangefinder() {
  static bool prevMode = false;  //上次模式
  if (th_mode) {
    if (!prevMode) {
      turnAllLedsOff();
      noTone(BUZZ);
    }
    breathingLED();
    //调整阈值
    static int old = encCnt;
    int diff = encCnt - old;
    if (diff > 2) thr++;
    else if (diff < -2) thr--;
    if (thr > 50) thr = 50;
    if (thr < 10) thr = 10;
    itoa(thr, th_str, 10);
    old = encCnt;
    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_7x14_tr);
      u8g2.drawStr(15, 12, "Threshold Adjust");
      u8g2.drawStr(20, 36, "Set:");
      u8g2.drawStr(55, 36, th_str);
      u8g2.drawStr(80, 36, "cm");
      u8g2.setFont(u8g2_font_6x10_tr);
      u8g2.drawStr(6, 56, "Long press Button 2 to exit");
    } while (u8g2.nextPage());
  } else {
    if (prevMode) turnAllLedsOff();
    unsigned int us = sonar.ping();
    float d = (float)us / 58.0f;  //d(cm) ≈ us / 58
    char dstr[12];
    bool show = false;  //是否显示
    // 无回波或超过 1m，不报警不显示
    if (us == 0 || d >= 100.0f) {
      turnAllLedsOff();
      noTone(BUZZ);
      show = false;
    }
    // 报警/绿灯
    else {
      if (d > thr) largerthan(d);
      else smallerthan(d);
      show = true;
      //两位有效数字
      //>=10 显示整数
      if (d >= 10.0f) {
        float r = floorf(d + 0.5f);
        dtostrf(r, 2, 0, dstr);
      }
      //>=1 显示 1 位小数
      else if (d >= 1.0f) {
        float r = roundf(d * 10.0f) / 10.0f;
        dtostrf(r, 3, 1, dstr);
      }
      //>=0.10 显示 2 位小数
      else if (d >= 0.10f) {
        float r = roundf(d * 100.0f) / 100.0f;
        dtostrf(r, 4, 2, dstr);
      }
      //更小则显示“<0.10
      else {
        strcpy(dstr, "<0.10");
      }
    }
    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_7x14_tr);
      u8g2.drawStr(30, 12, "Rangefinder");
      u8g2.drawStr(10, 56, "Thr:");
      u8g2.drawStr(40, 56, th_str);
      u8g2.drawStr(60, 56, "cm");
      if (!show) u8g2.drawStr(20, 34, "Out of Range");
      else {
        u8g2.drawStr(64, 34, dstr);
        u8g2.drawStr(108, 34, "cm");
      }
    } while (u8g2.nextPage());
  }
  prevMode = th_mode;
}

//示波器 Ex2.7 坐标轴 Ex2.8 时域波形 Ex2.9 切换信道
void Oscillocope() {
  if (millis() - lastSamp >= SMP_INT_MS) {
    lastSamp = millis();  // 更新采样时间
    for (int i = 0; i < OSC_W - 1; i++) {
      waveBuf[i] = waveBuf[i + 1];
    }  //左移
    int aval = analogRead(ainPins[chIdx]);
    waveBuf[OSC_W - 1] = map(aval, 0, 1023, OSC_H - 1, 0);  //y从 [0,1023] 映射到 [OSC_H-1,0]
  }

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_7x14_tr);
    u8g2.drawStr(25, 10, "Oscilloscope");
    u8g2.setFont(u8g2_font_6x10_tr);  //显示通道
    u8g2.setCursor(40, 60);
    u8g2.print("Channel: A");
    u8g2.print(chIdx);
    // x 轴
    const byte ARW = 4;
    int y0 = OSC_H / 2;
    int x1 = OSC_X0 + OSC_W - 1;
    u8g2.drawHLine(OSC_X0, y0, OSC_W);
    u8g2.drawLine(x1, y0, x1 - ARW, y0 - ARW);
    u8g2.drawLine(x1, y0, x1 - ARW, y0 + ARW);
    // y 轴
    int x0 = OSC_X0, yt = 0;
    u8g2.drawVLine(x0, 0, OSC_H);
    u8g2.drawLine(x0, yt, x0 - ARW, yt + ARW);
    u8g2.drawLine(x0, yt, x0 + ARW, yt + ARW);

    for (int i = 0; i < OSC_W - 1; i++) {
      int xA = OSC_X0 + i;
      int yA = constrain(waveBuf[i], 0, OSC_H - 1);
      int xB = OSC_X0 + i + 1;
      int yB = constrain(waveBuf[i + 1], 0, OSC_H - 1);
      u8g2.drawLine(xA, yA, xB, yB);
    }
  } while (u8g2.nextPage());
}

//音乐播放器 Ex2.10 Ex3.3 Ex3.4 播放曲库中乐曲，添加播放进度条和节奏闪灯
void play() {
  // 暂停静音且 LED 不闪
  if (isPause) {
    noTone(BUZZ);
    if (ledBeatOn) {
      turnAllLedsOff();
      ledBeatOn = false;
    }
    return;
  }
  // 结束自动熄灭
  if (ledBeatOn && millis() >= ledOffAt) {
    turnAllLedsOff();
    ledBeatOn = false;
  }
  const int(*curSong)[2] = songs[songIdx];
  if (noteIdx == 0 && songBeg == 0) songBeg = millis();
  if (noteEndAt == 0 || millis() >= noteEndAt) {
    noTone(BUZZ);
    //播放结束退出
    if (curSong[noteIdx][0] == 0 && curSong[noteIdx][1] == 0) {
      noteIdx = 0;
      noteEndAt = 0;
      songBeg = 0;
      turnAllLedsOff();
      ledBeatOn = false;
      return;
    }
    int f = curSong[noteIdx][0];  // 频率
    int d = curSong[noteIdx][1];  // 时长
    if (f > 0) {
      tone(BUZZ, f);
      analogWrite(G_PIN, 255);  // 绿灯节拍
      ledBeatOn = true;
      ledOffAt = millis() + 100;  // 自动熄灭
    } else {
      noTone(BUZZ);
      turnAllLedsOff();
      ledBeatOn = false;
    }

    noteEndAt = millis() + d;
    noteIdx++;  //下一个音符
  }
}

void Music_Player() {
  play();
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_7x14_tr);
    u8g2.drawStr(35, 10, "Music Player");

    u8g2.setCursor(40, 30);
    if (isPause) u8g2.print("Paused");
    else u8g2.print("Playing");

    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.setCursor(5, 45);
    u8g2.print("Song: ");
    u8g2.print(songTitles[songIdx]);
    // 进度条
    int bx = 14, by = 55, bw = 100, bh = 8;  //设置左上角，宽，高
    u8g2.drawFrame(bx, by, bw, bh);
    unsigned long tot = songDur[songIdx];
    // elp = （暂停中）暂停 - 开始；（播放中）当前 - 开始
    unsigned long elp = isPause ? (pausAt - songBeg) : (millis() - songBeg);
    if (elp > tot) elp = tot;                          //防止超出总时长
    int pw = (tot > 0) ? map(elp, 0, tot, 0, bw) : 0;  // 把 elp 从 [0, tot] 映射到 [0, bw]
    u8g2.drawBox(bx, by, pw, bh);
  } while (u8g2.nextPage());
}

//按键
//单击按钮1：手电筒换挡
void Click1() {
  if (state == Flashlight_state) ledLvl++;
}

//单击按钮2
void Click2() {
  // 播放/暂停切换
  if (state == Music_Player_state) {
    isPause = !isPause;
    //暂停
    if (isPause) {
      noTone(BUZZ);
      turnAllLedsOff();
      analogWrite(B_PIN, 255);
      pausAt = millis();  // 暂停时间
      ledBeatOn = false;
    }
    //恢复：补偿暂停的时间，使进度条连续
    else {
      // 恢复播放：
      if (pausAt > 0) {
        songBeg += (millis() - pausAt);
        pausAt = 0;
      }
      noteEndAt = millis();  //下一音符
      turnAllLedsOff();
    }
  }
  // 示波器：切换通道
  else if (state == Oscillocope_state) {
    chIdx = (chIdx + 1) % chCnt;
    for (int i = 0; i < OSC_W; i++) waveBuf[i] = OSC_H / 2;
    lastSamp = millis();
  }
}

//长按按钮1：进入菜单
void LongPress1() {
  if (state == Animation_state) state = Menu_state;
}

//长按按钮2
void LongPress2() {
  // 测距仪：进入/退出阈值设置模式
  if (state == Rangefinder_state) {
    th_mode = !th_mode;
  }
  // 音乐播放器：切换下一首并复位状态
  else if (state == Music_Player_state) {
    songIdx = (songIdx + 1) % NUM_SONGS;
    noteIdx = 0;
    noteEndAt = 0;
    songBeg = 0;
    pausAt = 0;
    isPause = false;
    noTone(BUZZ);
    turnAllLedsOff();
    ledBeatOn = false;
  }
}

//双击按钮1
void DoubleClick1() {
  //选中菜单
  if (state == Menu_state) {
    state = (State)(holdItem + 2);  //从手电筒开始
    //如果是音乐播放器，则重置
    if (state == Music_Player_state) {
      resetMusicState();
    }
    //如果是示波器模式，初始化
    else if (state == Oscillocope_state) {
      chIdx = 0;
      for (int i = 0; i < OSC_W; i++) waveBuf[i] = OSC_H / 2;
      lastSamp = millis();
      turnAllLedsOff();
    }
    //否则，静音
    else {
      allQuiet();
      resetMusicState();
    }
  }
  // 从功能页返回菜单
  else if (state >= Flashlight_state) {
    state = Menu_state;
    allQuiet();
    resetMusicState();
  }
}

void setup() {
  u8g2.begin();
  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  pinMode(BUZZ, OUTPUT);
  sg.attach(SERVO);
  btn1.attachClick(Click1);
  btn1.attachDoubleClick(DoubleClick1);
  btn1.attachLongPressStart(LongPress1);
  btn1.setDebounceTicks(30);
  btn1.setClickTicks(250);
  btn2.attachClick(Click2);
  btn2.attachLongPressStart(LongPress2);
  attachInterrupt(0, encodercount, CHANGE);  //D2引脚变化，调用encodercount()
  // 初始化
  isPause = false;
  songIdx = 0;
  noteIdx = 0;
  noteEndAt = 0;
  songBeg = 0;
  pausAt = 0;
  ledBeatOn = false;
  turnAllLedsOff();
  for (int i = 0; i < OSC_W; i++) waveBuf[i] = OSC_H / 2;
  //计算歌曲总时长
  for (int s = 0; s < NUM_SONGS; s++) {
    unsigned long tot = 0;
    int n = 0;
    while (songs[s][n][0] != 0 || songs[s][n][1] != 0) {
      tot += songs[s][n][1];
      n++;
    }
    songDur[s] = tot;
  }
}

void loop() {
  switch (state) {
    case Animation_state: Animation(); break;
    case Menu_state: Menu(); break;
    case Flashlight_state: Flashlight(); break;
    case Voltage_Meter_state: Voltage_Meter(); break;
    case Rangefinder_state: Rangefinder(); break;
    case Oscillocope_state: Oscillocope(); break;
    case Music_Player_state: Music_Player(); break;
  }
  btn1.tick();
  btn2.tick();
}
