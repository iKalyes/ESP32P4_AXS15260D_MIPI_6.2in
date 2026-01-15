## 🖥️ ESP32-P4 6.2寸 AXS15260D MIPI DSI LCD 示例代码库

基于 ESP-IDF v5.5.2 的 6.2 寸 MIPI DSI LCD 显示和触摸屏示例代码库。    
基于厂家提供的ESP32P4示例代码修改，使用LVGL V9.4图形库，启用了Espiessif PPA图形加速功能。

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
| 🔄 LCD 复位 | GPIO5 |
| 💡 LCD 背光 | GPIO20 |
| 📡 触摸 SDA | GPIO7 |
| 📡 触摸 SCL | GPIO8 |
| 🔄 触摸复位 | GPIO6 |
| ⚡ 触摸中断 | GPIO21 |

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
## ⚙ ESP32P4 MIPI接口硬件参考（仅针对该屏幕）

1.屏幕RST引脚和触摸TP_RST引脚相连，因此只需要执行一次复位即可；    
2.屏幕VDD电源与触摸TP_VDD电源相连，最好使用同一个电源；    
3.屏幕的手册有误，如果使用ESP32P4驱动，屏幕IOVCC电源为3.3V供电才可正常工作；    
4.待添加。

<img width="853" height="726" alt="QQ20260116-015056" src="https://github.com/user-attachments/assets/8b17439f-bbb8-497f-83f9-e85a95eee049" />

## 🖥️ 实际显示效果

![IMG_20260116_013717](https://github.com/user-attachments/assets/5d478eca-10ea-4156-bb4d-b27f4452dbc6)


