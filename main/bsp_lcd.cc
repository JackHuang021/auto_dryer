/**
 * @file bsp_lcd.cc
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-07-01
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lvgl_port.h"

#include "bsp_lcd.h"
#include "config.h"

static const char *TAG = "lcd";

static esp_lcd_panel_io_handle_t panel_io_handle = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;
static lv_display_t *lvgl_disp = NULL;

lv_display_t *get_lvgl_display()
{
    return lvgl_disp;
}

static void lcd_cmd_init(void)
{
    esp_lcd_panel_io_tx_param(panel_io_handle, 0x11, NULL, 0);
    vTaskDelay(120 / portTICK_PERIOD_MS);

    esp_lcd_panel_io_tx_param(panel_io_handle, 0x36, (uint8_t[]) { 0x00 }, 1);
    esp_lcd_panel_io_tx_param(panel_io_handle, 0x3A, (uint8_t[]) { 0x05 }, 1);
    esp_lcd_panel_io_tx_param(panel_io_handle, 0xB2,
                              (uint8_t[]) { 0x1F, 0x1F, 0x00, 0x33, 0x33 }, 5);
    esp_lcd_panel_io_tx_param(panel_io_handle, 0xB7, (uint8_t[]) { 0x35 }, 1);
    esp_lcd_panel_io_tx_param(panel_io_handle, 0xBB, (uint8_t[]) { 0x2B }, 1);
    esp_lcd_panel_io_tx_param(panel_io_handle, 0xC0, (uint8_t[]) { 0x2C }, 1);
    esp_lcd_panel_io_tx_param(panel_io_handle, 0xC2, (uint8_t[]) { 0x01 }, 1);
    esp_lcd_panel_io_tx_param(panel_io_handle, 0xC3, (uint8_t[]) { 0x0F }, 1);
    esp_lcd_panel_io_tx_param(panel_io_handle, 0xC4, (uint8_t[]) { 0x20 }, 1);
    esp_lcd_panel_io_tx_param(panel_io_handle, 0xC6, (uint8_t[]) { 0x13 }, 1);
    esp_lcd_panel_io_tx_param(panel_io_handle, 0xD0,
                              (uint8_t[]) { 0xA4, 0xA1 }, 2);
    esp_lcd_panel_io_tx_param(panel_io_handle, 0xD6, (uint8_t[]) { 0xA1 }, 1);
    esp_lcd_panel_io_tx_param(panel_io_handle, 0xE0,
                              (uint8_t[]) { 0xF0, 0x04, 0x07, 0x04,
                                            0x04, 0x04, 0x25, 0x33,
                                            0x3C, 0x36, 0x14, 0x12,
                                            0x29, 0x30 }, 14);
    esp_lcd_panel_io_tx_param(panel_io_handle, 0xE1,
                              (uint8_t[]) { 0xF0, 0x02, 0x04, 0x05,
                                            0x05, 0x21, 0x25, 0x32,
                                            0x3B, 0x38, 0x12, 0x14,
                                            0x27, 0x31 }, 14);

    esp_lcd_panel_io_tx_param(panel_io_handle, 0xE4,
                              (uint8_t[]) { 0x1D, 0x00, 0x00 }, 3);
    esp_lcd_panel_io_tx_param(panel_io_handle, 0x21, NULL, 0);
}

esp_err_t lcd_init(void)
{
    esp_err_t ret = ESP_OK;

    /* lcd backlight */
    gpio_config_t bk_gpio_config = { };
    bk_gpio_config.pin_bit_mask = 1ULL << LCD_GPIO_BL;
    bk_gpio_config.mode = GPIO_MODE_OUTPUT;

    ret = gpio_config(&bk_gpio_config);
    ESP_RETURN_ON_ERROR(ret, TAG, "lcd backlight gpio config failed");

    /* lcd init */
    spi_bus_config_t spi_bus_config = { };
    spi_bus_config.mosi_io_num = LCD_GPIO_MOSI;
    spi_bus_config.miso_io_num = GPIO_NUM_NC;
    spi_bus_config.sclk_io_num = LCD_GPIO_SCLK;
    spi_bus_config.quadwp_io_num = GPIO_NUM_NC;
    spi_bus_config.quadhd_io_num = GPIO_NUM_NC;
    spi_bus_config.max_transfer_sz = 
            LCD_WIDTH * LCD_DRAW_BUFF_HEIGHT * sizeof(uint16_t);

    ret = spi_bus_initialize(LCD_SPI_NUM, &spi_bus_config, SPI_DMA_CH_AUTO);
    ESP_RETURN_ON_ERROR(ret, TAG, "spi bus init failed");

    esp_lcd_panel_io_spi_config_t panel_io_config = { };
    panel_io_config.cs_gpio_num = LCD_GPIO_CS;
    panel_io_config.dc_gpio_num = LCD_GPIO_DC;
    panel_io_config.spi_mode = 3;
    panel_io_config.pclk_hz = LCD_PIXEL_CLK_HZ;
    panel_io_config.trans_queue_depth = 10;
    panel_io_config.lcd_cmd_bits = LCD_CMD_BITS;
    panel_io_config.lcd_param_bits = LCD_PARAM_BITS;

    esp_lcd_panel_dev_config_t panel_config = { };
    panel_config.reset_gpio_num = LCD_GPIO_RST;
    panel_config.color_space = LCD_COLOR_SPACE;
    panel_config.bits_per_pixel = LCD_BITS_PER_PIXEL;
    ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_NUM,
                                   &panel_io_config, &panel_io_handle);
    ESP_GOTO_ON_ERROR(ret, err, TAG, "create lcd panel io failed");

    ret = esp_lcd_new_panel_st7789(panel_io_handle, &panel_config, &panel_handle);
    ESP_GOTO_ON_ERROR(ret, err, TAG, "create st7789 lcd panel failed");

    ret = esp_lcd_panel_reset(panel_handle);
    ESP_GOTO_ON_ERROR(ret, err, TAG, "faile to reset lcd");
    lcd_cmd_init();
    // esp_lcd_panel_mirror(panel_handle, false, false);
    ret = esp_lcd_panel_disp_on_off(panel_handle, true);
    ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to set lcd on");

    /* turn backlight on */
    ret = gpio_set_level(LCD_GPIO_BL, LCD_BL_ON_LEVEL);
    ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to turn backlight on");

    ESP_LOGI(TAG, "lcd init done");

    return ret;

err:
    if (panel_handle)
        esp_lcd_panel_del(panel_handle);

    if (panel_io_handle)
        esp_lcd_panel_io_del(panel_io_handle);

    spi_bus_free(LCD_SPI_NUM);

    return ret;
}

esp_err_t lvgl_init(void)
{
    esp_err_t ret = ESP_OK;

    lvgl_port_cfg_t lvgl_config = { };
    lvgl_config.task_priority = 4;
    lvgl_config.task_stack = 4096;
    lvgl_config.task_affinity = -1;
    lvgl_config.task_max_sleep_ms = 500;
    lvgl_config.timer_period_ms = 5;

    ret = lvgl_port_init(&lvgl_config);
    ESP_RETURN_ON_ERROR(ret, TAG, "lvgl port init failed");

    lvgl_port_display_cfg_t display_config = { };
    display_config.io_handle = panel_io_handle;
    display_config.panel_handle = panel_handle;
    display_config.buffer_size = LCD_WIDTH * LCD_DRAW_BUFF_HEIGHT;
    display_config.double_buffer = LCD_DRAW_BUFF_DOUBLE;
    display_config.hres = LCD_WIDTH;
    display_config.vres = LCD_HEIGHT;
    display_config.monochrome = false;
#if LVGL_VERSION_MAJOR >= 9
    display_config.color_format = LV_COLOR_FORMAT_RGB565;
#endif
    display_config.rotation.swap_xy = false;
    display_config.rotation.mirror_x = false;
    display_config.rotation.mirror_y = false;
    display_config.flags.buff_dma = true;
#if LVGL_VERSION_MAJOR >= 9
    display_config.flags.swap_bytes = true;
#endif

    lvgl_disp = lvgl_port_add_disp(&display_config);
    ESP_RETURN_ON_FALSE(lvgl_disp != NULL, ESP_ERR_NO_MEM, TAG, "lvgl failed to add display");

    ESP_LOGI(TAG, "lvgl init done");

    return ret;
}


