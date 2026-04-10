# GitHub 仓库上传指南

## 仓库结构概览

```
github/                                    <-- 将此文件夹作为GitHub仓库根目录
├── README.md                             # 主README（个人介绍+项目索引）
├── LICENSE                               # MIT许可证
├── .gitignore                            # Git忽略文件
├── docs/                                 # 通用文档（可选）
├── utils/                                # 工具脚本（可选）
├── projects/                             # 项目目录
│   ├── 01-ra6m5-robot-system/           # 核心项目1：RA6M5机器人
│   ├── 02-freertos-pixel-clock/         # 核心项目2：像素时钟
│   ├── 03-inverted-pendulum/            # 核心项目3：倒立摆
│   ├── 04-stepper-motor-driver/         # 核心项目4：步进电机
│   ├── 05-stm32-autonomous-car/         # 核心项目5：自动行驶小车
│   ├── 06-ccs-m0-smart-car/             # 核心项目6：CCS_M0小车
│   └── optional/                        # 可选附加项目
│       ├── 07-stm32-balance-car/
│       ├── 08-stm32-ball-balance/
│       ├── 09-stm32-gimbal-ball/
│       ├── 10-stm32-line-follower/
│       ├── 11-stm32-openmv-tracker/
│       ├── 12-rtos-multi-uart/
│       └── 13-openmv-communication/
```

## 上传步骤

### 方式一：使用 Git 命令行

```bash
# 1. 进入 github 目录
cd "C:\Users\y\Desktop\资料汇总\github"

# 2. 初始化Git仓库
git init

# 3. 添加所有文件
git add .

# 4. 提交
git commit -m "Initial commit: Embedded portfolio projects"

# 5. 连接远程仓库（替换为你的GitHub用户名）
git remote add origin https://github.com/YourUsername/Embedded-Portfolio.git

# 6. 推送
git push -u origin main
```

### 方式二：使用 GitHub Desktop

1. 打开 GitHub Desktop
2. File → Add local repository
3. 选择 `C:\Users\y\Desktop\资料汇总\github` 文件夹
4. 点击 "Publish repository"
5. 输入仓库名称（建议：`Embedded-Portfolio` 或 `Embedded-Projects`）
6. 勾选 "Keep this code private"（如需要私有）
7. 点击 "Publish Repository"

### 方式三：直接拖拽（最简单）

1. 在 GitHub 网站创建新仓库
2. 打开 `C:\Users\y\Desktop\资料汇总\github` 文件夹
3. 全选所有文件，压缩为 `github.zip`
4. 在 GitHub 仓库页面点击 "uploading an existing file"
5. 上传压缩包，然后解压（或解压后上传文件夹）

## 需要补充的文件

上传前，请根据你的实际情况补充以下文件：

### 1. 源代码文件 (firmware/)

将各项目的实际源代码复制到对应目录：

```
projects/01-ra6m5-robot-system/firmware/
├── src/                    <-- 复制你的C源代码
│   ├── main.c
│   ├── robot_control.c
│   └── ...
├── inc/                    <-- 复制你的头文件
│   ├── robot_control.h
│   └── ...
└── project/                <-- 复制工程文件
    └── e2studio_project/

projects/02-freertos-pixel-clock/firmware/
├── src/                    <-- 复制源代码
├── inc/                    <-- 复制头文件
└── project/                <-- Keil工程文件
    └── PixelClock.uvprojx
```

### 2. 硬件设计文件 (hardware/)

```
hardware/
├── schematics/             <-- 原理图PDF或图片
├── pcb/                    <-- PCB设计文件
├── 3d-models/              <-- 3D模型文件（如有）
└── bom/                    <-- 物料清单
```

### 3. 图片文件 (docs/images/)

```
docs/images/
├── robot_system.jpg        <-- 机器人实物图
├── pixel_clock.jpg         <-- 像素时钟实物图
├── pcb_design.png          <-- PCB设计截图
└── demo_video.gif          <-- 演示动画
```

### 4. 更新个人信息

编辑 `README.md`：
- 更新 GitHub 用户名链接
- 添加 LinkedIn（如有）
- 添加个人网站（如有）

## 隐私检查清单

上传前请确认：

- [ ] 代码中没有 WiFi 密码
- [ ] 代码中没有 API 密钥
- [ ] 代码中没有个人信息（家庭地址等）
- [ ] 没有公司专有代码
- [ ] 没有未开源的第三方库源代码

## 文件大小注意事项

如果仓库太大，可以考虑：

1. 只上传核心源代码，不上传编译生成的中间文件
2. 图片压缩后再上传（推荐 tinypng.com）
3. 大文件使用 Git LFS（Large File Storage）

## 推荐仓库名称

- `Embedded-Portfolio`
- `STM32-Projects`
- `Embedded-Systems-Portfolio`
- `HuYongbo-Embedded`

## 后续维护

上传后可以：

1. 启用 GitHub Pages（免费展示页面）
2. 添加 Topics 标签（如 `stm32`, `freertos`, `embedded`）
3. 完善各项目的 README
4. 添加演示视频链接

---

**提示**：你可以分批上传，先上传核心6个项目，可选项目可以后续再添加。
