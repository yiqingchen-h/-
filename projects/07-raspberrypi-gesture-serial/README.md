# 树莓派手势识别与串口转发系统

## 项目概述

| 属性 | 内容 |
|------|------|
| **项目类型** | 树莓派视觉识别与串口联动 |
| **运行平台** | Raspberry Pi |
| **主要功能** | 摄像头采集手势图像、识别手势结果、通过串口转发给下位机 |
| **识别方案** | MediaPipe Hands + OpenCV |
| **实现语言** | Python |

## 项目简介

该项目用于在树莓派端完成手势识别，并将识别结果通过串口发送给下位机。基于 `picamera2` 获取 CSI 摄像头画面，结合 `MediaPipe` 完成手部关键点识别，再根据关键点关系判断常见数字手势和若干扩展手势。

## 技术栈

- **平台**：Raspberry Pi
- **语言**：Python 3
- **图像采集**：picamera2、libcamera
- **视觉处理**：OpenCV、MediaPipe Hands
- **通信方式**：UART 串口转发

## 功能说明

### 已整理内容
- 摄像头图像采集
- 手部关键点检测
- 常见手势判断
- 识别结果画面叠加显示
- 串口发送接口预留
- 重复发送间隔控制

### 当前可识别手势
- Zero
- One
- Two
- Three
- Four
- Five
- Six
- Seven
- Eight
- OK
- Rock
- Thumb_up
- Thumb_down
- Heart_single

## 展示材料

- 实物图：[查看图片](./media/images/6f952b99f9af21c0538dd196726020b1.jpg)
- 测试视频：[查看视频](./media/videos/7d87af5621d5446ab0825edb90d0d4ba.mp4)

## 目录结构

```text
07-raspberrypi-gesture-serial/
├── media/
│   ├── images/
│   └── videos/
├── src/
│   └── eRecognition_CSI.py
└── README.md
```
