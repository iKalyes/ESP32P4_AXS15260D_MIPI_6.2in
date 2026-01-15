# 🖥️ ESP32-P4 6.2寸 AXS15260 MIPI DSI LCD 演示项目

基于 ESP-IDF v5.5 的 6.2 寸 MIPI DSI LCD 显示和触摸屏演示项目。

## 📋 硬件规格

| 参数 | 规格 |
|------|------|
| 🖥️ LCD 型号 | AXS15260D |
| 📐 分辨率 | 452 x 1280 |
| 📡 接口 | 2 Lane MIPI DSI |
| 🎨 色深 | RGB888 (24位) |
| ⏱️ 刷新率 | 60Hz |
| 👆 触摸屏 | 电容式 5 点触控 |
| 📡 触摸接口 | I2C (地址: 0x3B) |

## 🔌 引脚配置

| 功能 | GPIO |
|------|------|
| 🔄 LCD 复位 | GPIO24 |
| 💡 LCD 背光 | GPIO29 |
| 📡 触摸 SDA | GPIO26 |
| 📡 触摸 SCL | GPIO27 |
| 🔄 触摸复位 | GPIO28 |
| ⚡ 触摸中断 | GPIO25 |

## 📁 项目结构

```
├── components/
│   └── esp_lcd_axs15260/          # 🖥️ LCD 和触摸屏驱动组件
│       ├── include/
│       │   ├── esp_lcd_axs15260.h         # LCD 驱动头文件
│       │   └── esp_lcd_touch_axs15260.h   # 触摸屏驱动头文件
│       └── src/
│           ├── esp_lcd_axs15260.c         # LCD 驱动实现
│           └── esp_lcd_touch_axs15260.c   # 触摸屏驱动实现
├── main/
│   └── main.c                     # 🚀 主程序
└── managed_components/
    ├── espressif__esp_lvgl_port/  # 🎨 LVGL 移植层
    └── lvgl__lvgl/                # 🎨 LVGL 图形库
```

## 🚀 快速开始

### 1️⃣ 环境要求

- ESP-IDF v5.3+
- ESP32-P4 开发板

### 2️⃣ 编译

```bash
idf.py build
```

### 3️⃣ 烧录

```bash
idf.py -p /dev/ttyUSB0 -b 2000000 flash monitor
```

## 🔧 组件 API

### 🖥️ LCD 驱动

```c
#include "esp_lcd_axs15260.h"

// 创建面板
esp_lcd_new_panel_axs15260(io, &panel_cfg, &panel);

// 获取 DPI 面板句柄 (用于 LVGL)
esp_lcd_axs15260_get_dpi_panel(panel);
```

### 👆 触摸屏驱动

```c
#include "esp_lcd_touch_axs15260.h"

// 创建触摸屏
axs15260_touch_config_t cfg = {
    .i2c_sda = GPIO_NUM_26,
    .i2c_scl = GPIO_NUM_27,
    .rst_gpio = GPIO_NUM_28,
    .int_gpio = GPIO_NUM_25,
    .i2c_port = I2C_NUM_0,
};
axs15260_touch_new(&cfg, &touch);

// 读取触摸数据
if (axs15260_touch_is_pressed(touch)) {
    axs15260_touch_data_t data;
    axs15260_touch_read(touch, &data);
}
```

## 📝 更新日志

| 日期 | 描述 |
|------|------|
| 2025-01-09 | ✅ 添加触摸屏驱动，集成到组件 |
| 2025-01-09 | 🖥️ 初始版本，LCD 显示正常 |

## 👤 作者

**sila**

## 📄 许可证

Apache-2.0
