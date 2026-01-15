# 🖥️ ESP LCD AXS15260 组件

AXS15260 MIPI-DSI LCD 和触摸屏驱动组件，适用于 ESP-IDF v5.3+。

## 📋 特性

### 🖥️ LCD 驱动
- 分辨率: 452x1280
- 接口: 2 Lane MIPI DSI
- 刷新率: 60Hz
- 色深: RGB888 (24位)

### 👆 触摸屏驱动
- I2C 地址: 0x3B
- 最大触摸点: 5 点
- I2C 频率: 100kHz (可配置)
- 支持中断模式

## 📁 文件结构

```
esp_lcd_axs15260/
├── include/
│   ├── esp_lcd_axs15260.h          # 🖥️ LCD 驱动头文件
│   └── esp_lcd_touch_axs15260.h    # 👆 触摸屏驱动头文件
├── src/
│   ├── esp_lcd_axs15260.c          # 🖥️ LCD 驱动实现
│   └── esp_lcd_touch_axs15260.c    # 👆 触摸屏驱动实现
├── CMakeLists.txt
├── idf_component.yml
└── README.md
```

## 🚀 使用方法

### 🖥️ LCD 初始化

```c
#include "esp_lcd_axs15260.h"

// 创建 MIPI DSI 总线
esp_lcd_dsi_bus_config_t bus_cfg = AXS15260_PANEL_BUS_DSI_2CH_CONFIG();
esp_lcd_new_dsi_bus(&bus_cfg, &dsi_bus);

// 创建 DBI IO
esp_lcd_dbi_io_config_t dbi_cfg = AXS15260_PANEL_IO_DBI_CONFIG();
esp_lcd_new_panel_io_dbi(dsi_bus, &dbi_cfg, &mipi_io);

// 创建面板
esp_lcd_dpi_panel_config_t dpi_cfg = AXS15260_452_1280_PANEL_60HZ_DPI_CONFIG(LCD_COLOR_PIXEL_FORMAT_RGB888);
axs15260_vendor_config_t vendor_cfg = {
    .mipi_config = {
        .dsi_bus = dsi_bus,
        .dpi_config = &dpi_cfg,
        .lane_num = AXS15260_MIPI_LANES,
    },
    .flags.use_mipi_interface = 1,
};
esp_lcd_panel_dev_config_t panel_cfg = {
    .vendor_config = &vendor_cfg,
};
esp_lcd_new_panel_axs15260(mipi_io, &panel_cfg, &panel);
esp_lcd_panel_init(panel);
```

### 👆 触摸屏初始化

```c
#include "esp_lcd_touch_axs15260.h"

// 配置触摸屏
axs15260_touch_config_t cfg = {
    .i2c_sda = GPIO_NUM_26,
    .i2c_scl = GPIO_NUM_27,
    .rst_gpio = GPIO_NUM_28,
    .int_gpio = GPIO_NUM_25,
    .i2c_port = I2C_NUM_0,
};

// 创建触摸屏驱动
axs15260_touch_handle_t touch;
axs15260_touch_new(&cfg, &touch);

// 读取触摸数据
axs15260_touch_data_t data;
if (axs15260_touch_is_pressed(touch)) {
    axs15260_touch_read(touch, &data);
    if (data.point_num > 0) {
        printf("Touch: X=%d, Y=%d\n", data.points[0].x, data.points[0].y);
    }
}
```

### 🎨 与 LVGL 集成

```c
static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    if (!axs15260_touch_is_pressed(touch)) {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }
    
    axs15260_touch_data_t touch_data;
    if (axs15260_touch_read(touch, &touch_data) == ESP_OK && touch_data.point_num > 0) {
        data->point.x = touch_data.points[0].x;
        data->point.y = touch_data.points[0].y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

// 注册到 LVGL
lv_indev_t *indev = lv_indev_create();
lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
lv_indev_set_read_cb(indev, touch_read_cb);
```

## 🔧 API 参考

### LCD API

| 函数 | 描述 |
|------|------|
| `esp_lcd_new_panel_axs15260()` | 创建 LCD 面板 |
| `esp_lcd_axs15260_get_dpi_panel()` | 获取 DPI 面板句柄 |

### 触摸屏 API

| 函数 | 描述 |
|------|------|
| `axs15260_touch_new()` | 创建触摸屏驱动 |
| `axs15260_touch_del()` | 删除触摸屏驱动 |
| `axs15260_touch_reset()` | 复位触摸屏 |
| `axs15260_touch_read()` | 读取触摸数据 |
| `axs15260_touch_get_version()` | 读取固件版本 |
| `axs15260_touch_is_pressed()` | 检查是否有触摸 |
| `axs15260_touch_set_swap_xy()` | 设置坐标变换 |
| `axs15260_touch_register_cb()` | 注册中断回调 |

## 📝 更新日志

| 日期 | 版本 | 描述 |
|------|------|------|
| 2025-01-09 | 1.0.0 | 初始版本，支持 LCD 和触摸屏 |

## 📄 许可证

Apache-2.0
