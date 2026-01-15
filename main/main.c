/**
 * @file main.c
 * @brief 🚀 ESP32-P4 6.2寸 AXS15260 MIPI DSI LCD LVGL 演示程序
 * @note 分辨率: 452x1280, 2 Lane MIPI DSI, RGB888 24位色
 * @note 支持 AXS15260 触摸屏 (I2C 地址: 0x3B)
 * 
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_ldo_regulator.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 🖥️ LCD 和触摸屏驱动
#include "esp_lcd_axs15260.h"
#include "esp_lcd_touch_axs15260.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_mipi_dsi.h"

// 🎨 LVGL
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "demos/lv_demos.h"

static const char *TAG = "main";

// ============================================================================
// 🔧 硬件配置
// ============================================================================

// �️ LCD GPIO
#define LCD_RST_GPIO            GPIO_NUM_5
#define LCD_BL_GPIO             GPIO_NUM_20

// 👆 触摸屏 GPIO
#define TOUCH_I2C_SDA           GPIO_NUM_7
#define TOUCH_I2C_SCL           GPIO_NUM_8
#define TOUCH_RST_GPIO          GPIO_NUM_6
#define TOUCH_INT_GPIO          GPIO_NUM_21
#define TOUCH_I2C_PORT          I2C_NUM_0

// ⚡ MIPI DSI PHY 电源
#define MIPI_DSI_PHY_PWR_LDO_CHAN       3
#define MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV 2500

// 💡 背光 PWM
#define BL_LEDC_TIMER           LEDC_TIMER_0
#define BL_LEDC_MODE            LEDC_LOW_SPEED_MODE
#define BL_LEDC_CHANNEL         LEDC_CHANNEL_0
#define BL_LEDC_DUTY_RES        LEDC_TIMER_13_BIT
#define BL_LEDC_FREQ            5000
#define BL_LEDC_DUTY_MAX        8191

// ============================================================================
// 📦 全局变量
// ============================================================================

static esp_lcd_panel_handle_t s_panel = NULL;
static esp_lcd_panel_io_handle_t s_mipi_io = NULL;
static esp_lcd_dsi_bus_handle_t s_dsi_bus = NULL;
static axs15260_touch_handle_t s_touch = NULL;
static lv_indev_t *s_indev = NULL;

// ============================================================================
// 💡 背光控制
// ============================================================================

static esp_err_t backlight_init(void)
{
    ESP_LOGI(TAG, "💡 初始化背光...");
    
    ledc_timer_config_t timer = {
        .speed_mode = BL_LEDC_MODE,
        .timer_num = BL_LEDC_TIMER,
        .duty_resolution = BL_LEDC_DUTY_RES,
        .freq_hz = BL_LEDC_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_RETURN_ON_ERROR(ledc_timer_config(&timer), TAG, "❌ LEDC 定时器失败");

    ledc_channel_config_t channel = {
        .speed_mode = BL_LEDC_MODE,
        .channel = BL_LEDC_CHANNEL,
        .timer_sel = BL_LEDC_TIMER,
        .gpio_num = LCD_BL_GPIO,
        .duty = 0,
    };
    ESP_RETURN_ON_ERROR(ledc_channel_config(&channel), TAG, "❌ LEDC 通道失败");
    
    return ESP_OK;
}

static esp_err_t backlight_set(uint8_t percent)
{
    uint32_t duty = (BL_LEDC_DUTY_MAX * percent) / 100;
    ledc_set_duty(BL_LEDC_MODE, BL_LEDC_CHANNEL, duty);
    ledc_update_duty(BL_LEDC_MODE, BL_LEDC_CHANNEL);
    ESP_LOGI(TAG, "💡 背光: %d%%", percent);
    return ESP_OK;
}

// ============================================================================
// 🖥️ LCD 初始化
// ============================================================================

static esp_err_t lcd_init(void)
{
    // 💡 背光初始化
    ESP_RETURN_ON_ERROR(backlight_init(), TAG, "❌ 背光失败");
    backlight_set(0);

    // 🔄 LCD 复位
    ESP_LOGI(TAG, "🔄 LCD 复位...");
    gpio_config_t rst_cfg = {
        .pin_bit_mask = (1ULL << LCD_RST_GPIO),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&rst_cfg);
    gpio_set_level(LCD_RST_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(LCD_RST_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(170));

    // ⚡ MIPI DSI PHY 电源
    ESP_LOGI(TAG, "⚡ 启用 MIPI DSI PHY 电源...");
    esp_ldo_channel_handle_t ldo = NULL;
    esp_ldo_channel_config_t ldo_cfg = {
        .chan_id = MIPI_DSI_PHY_PWR_LDO_CHAN,
        .voltage_mv = MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV,
    };
    ESP_RETURN_ON_ERROR(esp_ldo_acquire_channel(&ldo_cfg, &ldo), TAG, "❌ LDO 失败");

    // 📡 MIPI DSI 总线
    ESP_LOGI(TAG, "📡 创建 MIPI DSI 总线...");
    esp_lcd_dsi_bus_config_t bus_cfg = AXS15260_PANEL_BUS_DSI_2CH_CONFIG();
    ESP_RETURN_ON_ERROR(esp_lcd_new_dsi_bus(&bus_cfg, &s_dsi_bus), TAG, "❌ DSI 总线失败");

    // 📡 MIPI DBI IO
    ESP_LOGI(TAG, "📡 创建 MIPI DBI IO...");
    esp_lcd_dbi_io_config_t dbi_cfg = AXS15260_PANEL_IO_DBI_CONFIG();
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_dbi(s_dsi_bus, &dbi_cfg, &s_mipi_io), TAG, "❌ DBI IO 失败");

    // 🖥️ AXS15260 面板
    ESP_LOGI(TAG, "🖥️ 创建 AXS15260 面板...");
    esp_lcd_dpi_panel_config_t dpi_cfg = AXS15260_452_1280_PANEL_60HZ_DPI_CONFIG(LCD_COLOR_PIXEL_FORMAT_RGB888);
    dpi_cfg.num_fbs = 2;
    dpi_cfg.in_color_format = LCD_COLOR_FMT_RGB888;
    dpi_cfg.out_color_format = LCD_COLOR_FMT_RGB888;

    axs15260_vendor_config_t vendor_cfg = {
        .mipi_config = {
            .dsi_bus = s_dsi_bus,
            .dpi_config = &dpi_cfg,
            .lane_num = AXS15260_MIPI_LANES,
        },
        .flags.use_mipi_interface = 1,
    };

    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = LCD_RST_GPIO,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 24,
        .vendor_config = &vendor_cfg,
    };

    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_axs15260(s_mipi_io, &panel_cfg, &s_panel), TAG, "❌ 面板创建失败");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(s_panel), TAG, "❌ 面板初始化失败");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(s_panel, true), TAG, "❌ 开启显示失败");

    ESP_LOGI(TAG, "✅ LCD 初始化完成 (%dx%d)", AXS15260_LCD_H_RES, AXS15260_LCD_V_RES);
    return ESP_OK;
}

// ============================================================================
// 👆 触摸屏
// ============================================================================

// 📍 保存上一次触摸坐标 (用于滑动)
static int16_t s_last_x = 0;
static int16_t s_last_y = 0;

static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    // 📖 读取触摸数据
    axs15260_touch_data_t touch;
    esp_err_t ret = axs15260_touch_read(s_touch, &touch);
    
    if (ret == ESP_OK && touch.point_num > 0) {
        // 👆 有触摸点
        s_last_x = touch.points[0].x;
        s_last_y = touch.points[0].y;
        data->point.x = s_last_x;
        data->point.y = s_last_y;
        
        // 📋 根据事件类型判断状态
        // event: 0=按下, 1=抬起, 2=接触/移动
        uint8_t event = touch.points[0].event;
        if (event == 1) {
            // ✋ 抬起事件
            data->state = LV_INDEV_STATE_RELEASED;
        } else {
            // 👆 按下或移动
            data->state = LV_INDEV_STATE_PRESSED;
            ESP_LOGI(TAG, "👆 触摸: X=%d, Y=%d", data->point.x, data->point.y);
        }
    } else {
        // ✋ 无触摸数据
        data->point.x = s_last_x;
        data->point.y = s_last_y;
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

static esp_err_t touch_init(void)
{
    ESP_LOGI(TAG, "👆 初始化触摸屏...");

    axs15260_touch_config_t cfg = {
        .i2c_sda = TOUCH_I2C_SDA,
        .i2c_scl = TOUCH_I2C_SCL,
        .rst_gpio = TOUCH_RST_GPIO,
        .int_gpio = TOUCH_INT_GPIO,
        .i2c_port = TOUCH_I2C_PORT,
    };

    esp_err_t ret = axs15260_touch_new(&cfg, &s_touch);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ 触摸屏初始化失败");
        return ret;
    }

    uint16_t ver = 0;
    if (axs15260_touch_get_version(s_touch, &ver) == ESP_OK) {
        ESP_LOGI(TAG, "👆 固件版本: 0x%04X", ver);
    }

    ESP_LOGI(TAG, "✅ 触摸屏初始化完成");
    return ESP_OK;
}

static esp_err_t touch_register_lvgl(void)
{
    s_indev = lv_indev_create();
    if (!s_indev) {
        ESP_LOGE(TAG, "❌ 创建输入设备失败");
        return ESP_FAIL;
    }
    lv_indev_set_type(s_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(s_indev, touch_read_cb);
    ESP_LOGI(TAG, "✅ 触摸屏已注册到 LVGL");
    return ESP_OK;
}

// ============================================================================
// 🎨 LVGL 初始化
// ============================================================================

static esp_err_t lvgl_init(void)
{
    ESP_LOGI(TAG, "🎨 初始化 LVGL...");
    
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "❌ LVGL Port 失败");

    esp_lcd_panel_handle_t dpi_panel = esp_lcd_axs15260_get_dpi_panel(s_panel);
    ESP_RETURN_ON_FALSE(dpi_panel, ESP_FAIL, TAG, "❌ 获取 DPI 面板失败");

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = s_mipi_io,
        .panel_handle = dpi_panel,
        .control_handle = s_panel,
        .buffer_size = AXS15260_LCD_H_RES * AXS15260_LCD_V_RES,
        .double_buffer = true,
        .hres = AXS15260_LCD_H_RES,
        .vres = AXS15260_LCD_V_RES,
        .color_format = LV_COLOR_FORMAT_RGB888,
        .flags.direct_mode = true,
    };

    const lvgl_port_display_dsi_cfg_t dsi_cfg = {
        .flags.avoid_tearing = true,
    };

    lv_display_t *disp = lvgl_port_add_disp_dsi(&disp_cfg, &dsi_cfg);
    ESP_RETURN_ON_FALSE(disp, ESP_FAIL, TAG, "❌ LVGL 显示注册失败");

    ESP_LOGI(TAG, "✅ LVGL 初始化完成");
    return ESP_OK;
}

// ============================================================================
// 🚀 主函数
// ============================================================================

void app_main(void)
{
    ESP_LOGI(TAG, "🚀 ESP32-P4 AXS15260 LVGL 演示程序");
    ESP_LOGI(TAG, "📋 分辨率: %dx%d RGB888", AXS15260_LCD_H_RES, AXS15260_LCD_V_RES);

    // 🖥️ LCD
    ESP_ERROR_CHECK(lcd_init());
    backlight_set(100);

    // 🎨 LVGL
    ESP_ERROR_CHECK(lvgl_init());
    

    // 👆 触摸屏
    if (touch_init() == ESP_OK) {
        if (lvgl_port_lock(0)) {
            touch_register_lvgl();
            lvgl_port_unlock();
        }
        ESP_LOGI(TAG, "✅ 触摸屏已启用");
    } else {
        ESP_LOGW(TAG, "⚠️ 触摸屏初始化失败");
    }

    // 🎨 LVGL Demo
    ESP_LOGI(TAG, "🎨 启动 LVGL Demo...");
    if (lvgl_port_lock(0)) {
        lv_demo_widgets();
        lvgl_port_unlock();
    }
    ESP_LOGI(TAG, "✅ 启动完成");

    // 📊 主循环
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        ESP_LOGI(TAG, "📊 堆内存: %lu 字节", (unsigned long)esp_get_free_heap_size());
    }
}
