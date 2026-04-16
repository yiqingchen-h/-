# 步进电机驱动系统

## 项目概述

| 属性 | 内容 |
|------|------|
| **开发时间** | 2024.06 - 2024.07 |
| **主控芯片** | STM32F103C8T6 |
| **驱动芯片** | TMC2209 |
| **通信协议** | UART (单线半双工) |
| **项目角色** | 独立开发 |

## 技术栈

- **MCU**: STM32F103C8T6
- **驱动器**: TMC2209 (Trinamic)
- **控制方式**: UART 协议配置 + 脉冲控制
- **细分设置**: 1/256 微步
- **校验算法**: CRC 校验
- **显示**: OLED 实时状态显示

## 核心功能

### TMC2209 UART 协议实现

```c
// TMC2209 寄存器配置
void TMC2209_Init(void);

// 单线 UART 通信
void TMC2209_WriteRegister(uint8_t reg, uint32_t data);
uint32_t TMC2209_ReadRegister(uint8_t reg);

// CRC 校验计算
uint8_t calcCRC(uint8_t* data, uint8_t len);
```

### 关键特性

| 特性 | 说明 |
|------|------|
| 微步细分 | 1/256 微步 (51200 步/转) |
| 控制精度 | 0.007°/步 |
| 通信速率 | 115200 bps |
| 静音模式 | StealthChop2 |
| 失速检测 | StallGuard4 |

## 硬件设计

- TMC2209 驱动模块
- STM32F103 控制板
- OLED 状态显示
- 42 步进电机

## 成果指标

| 指标 | 数值 |
|------|------|
| 控制精度 | 0.007°/步 |
| 通信可靠性 | CRC 校验保障 |
| 运行噪音 | 静音模式 |

## 目录结构

```
12-stepper-motor-driver/
├── firmware/           # 固件代码
│   ├── src/           # 源代码
│   ├── inc/           # 头文件
│   └── project/       # Keil 工程
├── docs/             # 文档
│   └── images/       # 实物照片
└── README.md         # 本文件
```




