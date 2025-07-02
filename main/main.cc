/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_event.h"

#include "config.h"
#include "bsp_aht20.h"
#include "Buzzer.h"
#include "iot_button.h"
#include "iot_knob.h"
#include "button_gpio.h"
#include "lvgl.h"
#include "bsp_lcd.h"
#include "ui.h"

static i2c_master_bus_handle_t i2c_bus_handle = NULL;

static const char *TAG = "main";
static Buzzer buz;
static button_handle_t btn_handle = NULL;
static knob_handle_t knob_handle = NULL;
static lv_indev_t *lv_indev = NULL;

static void lv_indev_read_cb(lv_indev_t * indev, lv_indev_data_t * data)
{
    int cnt_value = iot_knob_get_count_value(knob_handle);

    if (cnt_value != 0) {
        data->enc_diff = cnt_value;
        iot_knob_clear_count_value(knob_handle);
        data->state = LV_INDEV_STATE_RELEASED;
    } else {
        data->state = iot_button_get_key_level(btn_handle) ?
                    LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
        data->enc_diff = 0;
    }
}

static esp_err_t lvgl_indev_init(void)
{
    esp_err_t ret = ESP_OK;

    lv_indev = lv_indev_create();
    ESP_RETURN_ON_FALSE(lv_indev, ESP_ERR_NO_MEM, TAG, "not enough memory for lv_indev_t");
    lv_indev_set_type(lv_indev, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(lv_indev, lv_indev_read_cb);
    lv_indev_set_display(lv_indev, get_lvgl_display());

    return ret;
}


static esp_err_t i2c_init(void)
{
    esp_err_t ret = ESP_OK;

    i2c_master_bus_config_t i2c_bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_SDA_GPIO,
        .scl_io_num = I2C_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags = {
            .enable_internal_pullup = 1,
            .allow_pd = 0,
        },
    };

    ret = i2c_new_master_bus(&i2c_bus_config, &i2c_bus_handle);

    ESP_RETURN_ON_ERROR(ret, TAG, "failed to init i2c bus");

    ESP_LOGI(TAG, "i2c bus init done.");

    return ret;
}


static void button_press_down_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "BTN: BUTTON_PRESS_DOWN");
}

static void knob_right_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "KONB: KONB_RIGHT, count_value:%d",
             iot_knob_get_count_value((button_handle_t)arg));
}

static void knob_left_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "KONB: KONB_LEFT, count_value:%d",
             iot_knob_get_count_value((button_handle_t)arg));
}

static esp_err_t ec11_init(void)
{
    esp_err_t ret = ESP_OK;

    button_config_t btn_cfg = {
        .long_press_time = BUTTON_LONG_PRESS_TIME_MS,
        .short_press_time = BUTTON_SHORT_PRESS_TIME_MS,
    };

    button_gpio_config_t gpio_cfg = {
        .gpio_num = EC11_BTN_GPIO,
        .active_level = EC11_BTN_ACTIVE_LEVEL,
        .enable_power_save = true,
        .disable_pull = false,
    };

    ret = iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &btn_handle);
    ESP_RETURN_ON_ERROR(ret, TAG, "init button failed");
    ret = iot_button_register_cb(btn_handle, BUTTON_PRESS_DOWN, NULL, button_press_down_cb, NULL);
    ESP_RETURN_ON_ERROR(ret, TAG, "failed to register BUTTON_PRESS_DOWN cb");

    knob_config_t knob_config = {
        .default_direction = 0,
        .gpio_encoder_a = EC11_A_GPIO,
        .gpio_encoder_b = EC11_B_GPIO,
        .enable_power_save = true,
    };

    knob_handle = iot_knob_create(&knob_config);
    if (NULL == knob_handle) {
        ESP_LOGE(TAG, "knob create failed");
    }

    ret = iot_knob_register_cb(knob_handle, KNOB_LEFT, knob_left_cb, NULL);
    ESP_RETURN_ON_ERROR(ret, TAG, "failed to register KNOB_LEFT cb");
    ret = iot_knob_register_cb(knob_handle, KNOB_RIGHT, knob_right_cb, NULL);
    ESP_RETURN_ON_ERROR(ret, TAG, "failed to register KNOB_RIGHT cb");

    return ret;
}


extern "C" void app_main(void)
{
    esp_err_t ret = ESP_OK;
    // Initialize the default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize NVS flash for WiFi configuration
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "Erasing NVS flash to fix corruption");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    i2c_init();
    Aht20 aht20(i2c_bus_handle, AHT20_ADDRESS_0);
    aht20.init();

    lcd_init();
    lvgl_init();

    ec11_init();
    lvgl_indev_init();

    buz.Init(BEEP_GPIO);
    buz.Beep({.frequency = 1000, .duration_ms = 1000, .volume = 0.01f});
    vTaskDelay(pdMS_TO_TICKS(1000));
    buz.Play({{800, 200, 0.01f}, {500, 200, 0.01f}});

    HeaterUI ui;
    ui.init();
    ui.update(31, 45, 9, 52);

    while (1) {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        aht20.update();
        aht20.print();
    }

    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
