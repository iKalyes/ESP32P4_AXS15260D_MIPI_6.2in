# 🚀 Main 组件 CMakeLists.txt
idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_timer esp_lcd_axs15260 esp_lcd driver)
