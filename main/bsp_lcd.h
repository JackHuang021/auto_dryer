/**
 * @file bsp_lcd.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-07-01
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_display_t *get_lvgl_display();
esp_err_t lcd_init(void);
esp_err_t lvgl_init(void);


#ifdef __cplusplus
}
#endif
