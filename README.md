# 碧水哨兵 — 基于 STM32 的企业污水排放口达标监测与预警系统

> 物联网创新设计大赛参赛项目 · 低成本物联网排污监管终端

## 项目简介

本项目聚焦**企业污水排放口达标监测**场景，基于 STM32F103C8T6 单片机设计低成本物联网水质监测终端。实时采集排污口 pH、水温、电导率/TDS 等核心指标，通过 WiFi（ESP8266）或 4G 物联网卡接入云平台，支持「区域—企业—监测节点」三级管理，超标时自动报警并预留排污阻断接口。

### 政策依据

- 《排污许可管理办法》（2024）：排污单位应依法开展自行监测，原始记录保存不少于 5 年
- GB 8978-1996《污水综合排放标准》：pH 限值 6—9（一切排污单位）
- HJ 355-2019《水污染源在线监测系统运行技术规范》：pH、温度至少每 10 分钟一组数据

---

## 功能特性

- [x] STM32F103C8T6 主控，72MHz，HAL 库开发
- [x] pH 采集（PH4502C 模块 + 电极，两点校准）
- [x] 水温采集（DS18B20 数字温度传感器）
- [x] 电导率/TDS 采集（模拟量 TDS 模块）
- [x] OLED 本地显示（0.96 寸 SSD1306，I2C）
- [x] 超标声光报警（蜂鸣器 + LED）
- [x] WiFi 无线上报（ESP8266 + MQTT）
- [ ] 云平台三级管理界面（开发中）
- [ ] 双节点试点验证（计划中）
- [ ] 4G 物联网卡接入（预留接口）
- [ ] 自动药剂注入 / 智能阀控（预留扩展）

---

## 硬件清单

| 类别 | 元器件 | 型号 | 数量 | 参考单价 |
|------|--------|------|------|---------|
| 主控 | STM32 最小系统板 | STM32F103C8T6（蓝色 Pill 板） | 1 | ¥18 |
| 联网 | WiFi 模块 | ESP8266-01S | 1 | ¥12 |
| 传感器 | pH 模块 + 电极 | PH4502C + E-201-C | 1 | ¥25 |
| 传感器 | 水温传感器 | DS18B20 防水型 | 1 | ¥5 |
| 传感器 | TDS 模块 | TDS Meter v1.0 | 1 | ¥15 |
| 显示 | OLED 屏 | 0.96 寸 SSD1306 I2C | 1 | ¥10 |
| 报警 | 有源蜂鸣器 | 5V 有源 | 1 | ¥2 |
| 报警 | LED + 电阻 | 5mm 红/绿 LED + 220Ω | 2 | ¥1 |
| 耗材 | pH 校准液 | pH 4.00 / 6.86 / 9.18 | 各 1 | ¥15 |
| 杂项 | 面包板、杜邦线、电阻 | — | 若干 | ¥20 |

**单节点核心成本约 ¥120—150**（不含电源、外壳、4G 模组）

---

## 引脚定义

### 已配置引脚

| 引脚 | 功能 | 连接对象 | 方向 |
|------|------|---------|------|
| PA0 | ADC1_IN0 | PH4502C 模块 Po 端（pH 模拟输出） | 模拟输入 |
| PA1 | ADC1_IN1 | TDS 模块 AOUT 端（TDS 模拟输出） | 模拟输入 |
| PA2 | USART2_TX | ESP8266 RX | 复用推挽输出 |
| PA3 | USART2_RX | ESP8266 TX | 浮空输入 |
| PA5 | GPIO_Input | DS18B20 DATA（单总线，需 4.7kΩ 上拉到 3.3V） | 上拉输入 |
| PA9 | USART1_TX | USB 转 TTL RX（调试串口） | 复用推挽输出 |
| PA10 | USART1_RX | USB 转 TTL TX（调试串口） | 浮空输入 |
| PB8 | I2C1_SCL | OLED SCL（I2C1 重映射） | 复用开漏输出 |
| PB9 | I2C1_SDA | OLED SDA（I2C1 重映射） | 复用开漏输出 |
| PB12 | GPIO_Output | 有源蜂鸣器（经 S8050 三极管驱动） | 推挽输出，默认低 |
| PB13 | GPIO_Output | 报警 LED（串联 220Ω 电阻） | 推挽输出，默认低 |
| PC13 | GPIO_Output | 板载 LED（调试用） | 推挽输出，默认低 |
| PA13 | SYS_JTMS-SWDIO | ST-Link SWDIO | 调试接口 |
| PA14 | SYS_JTCK-SWCLK | ST-Link SWCLK | 调试接口 |

### 电源引脚

| 引脚 | 说明 |
|------|------|
| 5V | 接电源适配器 5V 正极，同时给 PH4502C、蜂鸣器供电 |
| 3.3V | STM32 逻辑、ESP8266、OLED、DS18B20 供电 |
| GND | 所有模块共地（必须共地，否则 ADC 读数不准） |

### 注意事项

- DS18B20 的 DATA 引脚与 3.3V 之间必须接 **4.7kΩ 上拉电阻**（购买的转接板已带）
- PH4502C 模块需 **5V 供电**，输出模拟信号到 PA0
- ESP8266 峰值电流 300mA，建议独立 3.3V LDO 或在 VCC 端加 100μF 电容
- pH 电极不可长时间干放，不用时浸泡在 KCl 保存液或蒸馏水中

### 传感器接线配对表（按此接线）

| 模块 | 模块引脚 | 接 STM32 / 电源 | 说明 |
|------|---------|----------------|------|
| **pH 模块**（PH4502C） | VCC | 5V | 需 5V 供电 |
| | GND | GND | 共地 |
| | PO | **PA0** (ADC1_IN0) | 模拟输出 0~3.0V，可直接接 |
| | DO | 不接 | 阈值输出，未使用 |
| **TDS 模块** | VCC | 3.3V ~ 5V | 按模块标称 |
| | GND | GND | 共地 |
| | AO | **PA1** (ADC1_IN1) | 模拟输出 0~2.3V，可直接接 |
| **DS18B20**（防水探头） | 红线 | 3.3V | |
| | 黑线 | GND | |
| | 黄线（DATA） | **PA5** | 转接板已带上拉电阻 |
| **OLED**（0.96 寸 I2C） | VCC | 3.3V | |
| | GND | GND | |
| | SCL | **PB8** | I2C1_SCL（重映射） |
| | SDA | **PB9** | I2C1_SDA（重映射） |
| **ESP8266-01S** | VCC | **3.3V** | ⚠️ 禁止接 5V，会烧毁 |
| | GND | GND | |
| | TX | **PA3** (USART2_RX) | 交叉连接 |
| | RX | **PA2** (USART2_TX) | 交叉连接 |
| | CH_PD / EN | **3.3V** | ⚠️ 必须接高电平，悬空不工作 |
| **蜂鸣器**（有源） | 信号 | **PB12** | 经三极管驱动 |
| **报警 LED** | 正极 | **PB13**（串联 220Ω） | 负极接 GND |
| **板载 LED** | — | **PC13** | 调试用 |

---

## 环境搭建

### 必需软件

| 软件 | 用途 | 下载地址 |
|------|------|---------|
| STM32CubeMX | 图形化配置芯片、生成初始化代码 | [st.com](https://www.st.com/en/development-tools/stm32cubemx.html) |
| VS Code | 代码编辑器 | [code.visualstudio.com](https://code.visualstudio.com/) |
| STM32CubeCLT | 命令行工具集（含 ARM-GCC 编译器 + OpenOCD 调试器） | [st.com](https://www.st.com/en/development-tools/stm32cubeclt.html) |
| Git | 版本控制 | [git-scm.com](https://git-scm.com/) |

### STM32CubeMX 配置教程（视频）

| 教程 | 说明 | 链接 |
|------|------|------|
| 如何使用 STM32CubeMX 快速配置工程【简单入门】 | 电子科技大学自动化学院创客中心 · 25 分钟手把手入门 | [B 站 BV1EBpxepEec](https://www.bilibili.com/video/BV1EBpxepEec/) |

### VS Code 插件

在 VS Code 扩展商店搜索并安装以下插件（均为 ST 官方）：

1. **STM32Cube Core** — 核心插件，项目识别和管理
2. **STM32Cube CMake Support** — CMake 构建支持
3. **STM32Cube Build Analysis** — 编译分析
4. **STM32Cube clangd** — 代码补全和跳转
5. **STM32Cube Bundles** — 组件包管理
6. **C/C++ Extension Pack**（Microsoft）— 代码补全和语法高亮

> 安装 STM32CubeCLT 时务必勾选 **"Add to PATH"**，否则 VS Code 找不到编译器。

---

## 下载与编译

### 1. 克隆项目

```bash
git clone https://github.com/mqiuyqiu/water-quality-monitoring.git
cd water-quality-monitoring
```

### 2. VS Code 打开项目

1. 打开 VS Code → 文件 → 打开文件夹 → 选择 `water-quality-monitoring` 目录
2. 左下角 STM32 插件会自动识别项目，等待 CMake 初始化完成
3. 底部状态栏会显示芯片型号（STM32F103C8Tx）和构建配置（Debug）

### 3. 编译

- 底部状态栏点 **Build**（或按 `Ctrl+Shift+B`）
- 选择 **Debug** 配置
- 终端输出 `Build complete` + 内存占用报告 = 编译成功
- 生成的固件在 `build/Debug/` 目录下：
  - `Water_quality_monitoring.elf` — 用于调试
  - `Water_quality_monitoring.bin` — 用于烧录
  - `Water_quality_monitoring.hex` — 用于烧录

### 常见编译问题

| 问题 | 解决方法 |
|------|---------|
| `arm-none-eabi-gcc not found` | STM32CubeCLT 没装或没加 PATH，重装时勾选 Add to PATH |
| CMake 初始化卡住 | 按 `Ctrl+Shift+P` → 输入 "STM32: Rebuild" 手动触发 |
| 头文件找不到 | 等 clangd 索引完成（右下角有进度条），或重启 VS Code |

---

## 烧录与调试

### 硬件连接（ST-Link）

| ST-Link | STM32F103 |
|---------|-----------|
| SWDIO | PA13 |
| SWCLK | PA14 |
| GND | GND |
| 3.3V | 3.3V（或板子外部供电） |

> 如果板子用外部电源供电，ST-Link 的 3.3V 可以不接，但 **GND 必须共地**。

### 烧录

1. ST-Link 插电脑 USB 口
2. VS Code 底部状态栏点 **Run**（三角形播放图标）
3. 自动编译 → 烧录 → 进入调试模式
4. 烧录成功后程序自动运行

### 调试

- 顶部调试工具栏：
  - `F5` 继续运行
  - `F10` 单步跳过
  - `F11` 单步进入
  - `Shift+F5` 停止调试
- 在代码行号左侧点一下设置断点
- 左侧变量面板查看实时变量值
- 串口调试：USB 转 TTL 接 PA9/PA10，用串口助手（115200 波特率）查看 printf 输出

---

## 项目结构

```
water-quality-monitoring/
├── Core/
│   ├── Inc/              # 头文件
│   │   ├── main.h
│   │   ├── gpio.h
│   │   ├── adc.h          # ADC 初始化（pH/TDS）
│   │   ├── usart.h        # 串口初始化（USART1 调试 + USART2 ESP8266）
│   │   └── i2c.h          # I2C 初始化（OLED）
│   └── Src/              # 源文件
│       ├── main.c         # 主程序（用户代码写在 USER CODE 区域）
│       ├── gpio.c
│       ├── adc.c
│       ├── usart.c
│       ├── i2c.c
│       └── stm32f1xx_it.c # 中断处理
├── Drivers/              # HAL 库（自动生成，不要手动改）
│   ├── CMSIS/
│   └── STM32F1xx_HAL_Driver/
├── cmake/                # CMake 工具链配置
├── CMakeLists.txt        # 构建配置
├── CMakePresets.json     # 构建预设
├── startup_stm32f103xb.s # 启动文件
├── STM32F103xx_FLASH.ld  # 链接脚本
├── Water_quality_monitoring.ioc  # CubeMX 配置文件（图形化配置用这个打开）
└── .gitignore
```

---

## 开发规范

### 用户代码只能写在 USER CODE 区域

CubeMX 重新生成代码时，只有 `/* USER CODE BEGIN */` 和 `/* USER CODE END */` 之间的代码会保留，其他区域会被覆盖：

```c
/* USER CODE BEGIN 2 */
// 在这里写你的初始化代码
HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
/* USER CODE END 2 */

while (1)
{
  /* USER CODE BEGIN 3 */
  // 在这里写主循环代码
  /* USER CODE END 3 */
}
```

### 自己新建的文件

- 源文件放 `Core/Src/`，头文件放 `Core/Inc/`
- 新建的 `.c` 文件需要在 `CMakeLists.txt` 里添加（CubeMX 重新生成时会自动扫描，但手动确认一下）
- 自己的文件不会被 CubeMX 覆盖

### 修改 CubeMX 配置

1. 双击 `Water_quality_monitoring.ioc` 用 CubeMX 打开
2. 修改引脚或外设配置
3. 点 **GENERATE CODE** 重新生成
4. 回 VS Code，用户代码保留在 USER CODE 区域

---

## 开发路线图

| 阶段 | 时间 | 任务 | 状态 |
|------|------|------|------|
| W1—W2 | 第 1—2 周 | 环境搭建、点灯、串口打印 | 进行中 |
| W3—W5 | 第 3—5 周 | ADC 采集 pH/TDS、DS18B20 水温、OLED 显示、pH 校准 | 待开始 |
| W6—W8 | 第 6—8 周 | ESP8266 连 WiFi、MQTT 上报巴法云、超标报警 | 待开始 |
| W9—W10 | 第 9—10 周 | 双节点搭建、云端多节点对比 | 待开始 |
| W11—W12 | 第 11—12 周 | 演示视频、答辩 PPT、模拟答辩 | 待开始 |

---

## 技术指标

| 指标 | 参数 |
|------|------|
| 主控 | STM32F103C8T6，Cortex-M3，72MHz |
| 供电 | 5V/2A（USB 或电源适配器） |
| pH 测量范围 | 0—14，精度 ±0.1（校准后） |
| 水温测量范围 | -55—125°C，精度 ±0.5°C |
| TDS 测量范围 | 0—1000 ppm |
| 采集频率 | 每 10 分钟一组（对标 HJ 355） |
| 通信方式 | WiFi（ESP8266）/ 4G（预留） |
| 通信协议 | MQTT |
| 报警方式 | 蜂鸣器 + LED + 云端消息推送 |
| 单节点成本 | 约 ¥150—250 |

---

## 许可证

本项目为学生竞赛项目，仅供学习和交流使用。

---

## 团队

- 项目名称：碧水哨兵
- 参赛赛事：物联网创新设计大赛
- 指导老师：（待填写）
- 团队成员：（待填写）
