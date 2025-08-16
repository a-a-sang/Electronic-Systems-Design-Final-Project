#pragma once
#include <Arduino.h>
#define NUM_SONGS 5

// 曲名数组
extern const char* songTitles[NUM_SONGS];

// 每首歌都是一个 int[][2]（{频率Hz, 时长ms}），以 {0,0} 结尾
extern const int (*const songs[NUM_SONGS])[2];
