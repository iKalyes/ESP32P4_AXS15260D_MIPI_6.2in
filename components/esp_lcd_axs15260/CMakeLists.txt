# 🖥️ ESP LCD AXS15260 组件 CMakeLists.txt
# SPDX-FileCopyrightText: 2025
# SPDX-License-Identifier: Apache-2.0

idf_component_register(
    SRCS "src/esp_lcd_axs15260.c"
         "src/esp_lcd_touch_axs15260.c"
    INCLUDE_DIRS "include"
    REQUIRES "driver" "esp_lcd"
    PRIV_REQUIRES "esp_mm"
)
