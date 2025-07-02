/**
 * @file config.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-06-28
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef __CONFIG_H_
#define __CONFIG_H_

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_types.h"

#define I2C_SCL_GPIO    (GPIO_NUM_3)
#define I2C_SDA_GPIO    (GPIO_NUM_4)

#define BEEP_GPIO       (GPIO_NUM_1)
#define PTC_GPIO        (GPIO_NUM_0)

#define EC11_BTN_GPIO   (GPIO_NUM_10)
#define EC11_A_GPIO     (GPIO_NUM_20)
#define EC11_B_GPIO     (GPIO_NUM_21)

/* LCD pins */
#define LCD_GPIO_SCLK       (GPIO_NUM_5)
#define LCD_GPIO_MOSI       (GPIO_NUM_6)
#define LCD_GPIO_RST        (GPIO_NUM_2)
#define LCD_GPIO_DC         (GPIO_NUM_7)
#define LCD_GPIO_CS         (GPIO_NUM_NC)
#define LCD_GPIO_BL         (GPIO_NUM_8)

/* LCD settings */
#define LCD_SPI_NUM             (SPI2_HOST)
#define LCD_PIXEL_CLK_HZ        (40 * 1000 * 1000)
#define LCD_CMD_BITS            (8)
#define LCD_PARAM_BITS          (8)
#define LCD_COLOR_SPACE         (ESP_LCD_COLOR_SPACE_BGR)
#define LCD_BITS_PER_PIXEL      (16)
#define LCD_DRAW_BUFF_DOUBLE    (1)
#define LCD_DRAW_BUFF_HEIGHT    (40)
#define LCD_BL_ON_LEVEL         (1)
#define LCD_WIDTH               (240)
#define LCD_HEIGHT              (240)

#define EC11_BTN_ACTIVE_LEVEL   (0)

#define DEFAULT_I2C_CLOCK       (100000)

#endif
