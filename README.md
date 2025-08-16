# 清华大学电子系2025夏季学期电子系统专题设计大作业
# Electronic Systems Final Project

## Overview

This repository contains the final project for the course **Electronic Systems Design and Implementation**. 
The project integrates multiple hardware modules (LED, OLED, servo, ultrasonic sensor, motor driver, buzzer, encoder, etc.) with Arduino-based programming to realize a multifunctional electronic system.

## Features

The system is divided into multiple functional states, navigated through a menu interface using buttons and a rotary encoder.  
Key modules include:

- **3D Menu System**  
  - Rotary encoder navigation with smooth animation and highlight effects  
  - Startup animation with welcome message

- **Flashlight Mode**  
  - Multi-level brightness control (blue dim → white dim → white bright) using RGB LEDs

- **Voltage Meter**  
  - Real-time voltage display on OLED (2 decimal precision)  
  - Servo pointer mapped from 0–5V to 10–170°  
  - On-screen pointer visualization

- **Ultrasonic Rangefinder**  
  - Distance display (cm, 2 significant digits)  
  - Adjustable alarm threshold (10–50 cm)  
  - Dynamic LED + buzzer warning when distance < threshold

- **Oscilloscope**  
  - Vertical axes and real-time waveform display from analog channels  
  - Channel switching between A0, A1, A2…

- **Music Player**  
  - Preloaded melodies (e.g., *Dong Fang Hong*) with buzzer playback  
  - Progress bar visualization  
  - Beat-synchronized LED flashing  
  - Song switching and pause/resume

## Hardware Components

- Arduino Uno / Mega 2560
- RGB LED
- OLED display (SH1106 driver)
- Servo motor
- Ultrasonic sensor
- DC motor + encoder
- Motor driver module
- Buzzer
- Push buttons (x2)
- Rotary encoder

## Software Highlights

- Modular **state machine** structure for different modes
- **Debounced button handling** with `OneButton` library (single/double/long press actions)
- **Efficient drawing routines** to maintain smooth UI updates
- **PWM control** for LEDs and servo positioning
- **Analog and digital signal processing** for measurement modules

## Acknowledgements

Special thanks to the course instructors and TAs for their guidance, and to all peers for their collaboration and creative inspiration during the project.

---


## 项目简介

本仓库为 **《电子系统设计与实现》课程** 的大作业项目。  
本项目基于 Arduino 平台，将 LED、OLED、舵机、超声波传感器、电机驱动、蜂鸣器、编码器等多种硬件模块整合，实现了多功能电子系统。

## 功能概览

系统采用多功能状态机结构，通过按钮与旋转编码器在菜单界面中进行切换与操作。  
主要功能包括：

- **3D 菜单系统**  
  - 旋转编码器导航，平滑动画与高亮效果  
  - 开机动画与欢迎信息

- **手电筒模式**  
  - 多档亮度控制（蓝色微亮 → 白色微亮 → 白色最亮），基于 RGB LED PWM 调节

- **电压表模式**  
  - OLED 实时显示电压值（精确到小数点后两位）  
  - 舵机指针将 0–5V 映射到 10–170°  
  - 屏幕同步显示指针

- **超声波测距模式**  
  - 实时显示距离（cm，保留两位有效数字）  
  - 阈值可调（10–50 cm）  
  - 距离小于阈值时红灯闪烁 + 蜂鸣器报警，报警频率随距离增大而加快

- **示波器模式**  
  - 屏幕显示垂直坐标轴和实时波形  
  - 可切换通道 A0、A1、A2…

- **音乐播放器模式**  
  - 预存曲目（如《东方红》）并用蜂鸣器播放  
  - 播放进度条显示  
  - 节拍同步 LED 闪烁  
  - 支持切歌、暂停/恢复播放

## 硬件组成

- Arduino Uno / Mega 2560
- RGB LED
- OLED 显示屏（SH1106 驱动）
- 舵机
- 超声波传感器
- 带编码器的直流电机
- 电机驱动模块
- 蜂鸣器
- 按钮（2 个）
- 旋转编码器

## 软件亮点

- 模块化 **状态机** 结构，支持多模式切换
- 使用 `OneButton` 库实现防抖处理（单击/双击/长按）
- 高效绘图与界面刷新逻辑，保证流畅显示
- PWM 控制实现 LED 亮度与舵机位置控制
- 模拟与数字信号采集、处理

## 致谢

特别感谢课程教师与助教的指导，以及在项目过程中提供灵感与帮助的同学们。
