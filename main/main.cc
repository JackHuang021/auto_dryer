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


static i2c_master_bus_handle_t i2c_bus_handle = NULL;

static const char *TAG = "main";

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

static esp_err_t beep_init(void)
{
    esp_err_t ret = ESP_OK;

    gpio_config_t io_conf = {
        .pin_bit_mask = BEEP_GPIO,              // 选择GPIO
        .mode = GPIO_MODE_OUTPUT,               // 设置为输出模式
        .pull_up_en = GPIO_PULLUP_DISABLE,      // 不启用上拉
        .pull_down_en = GPIO_PULLDOWN_DISABLE,  // 不启用下拉
        .intr_type = GPIO_INTR_DISABLE          // 不使能中断
    };
    ret = gpio_config(&io_conf);
    ret = gpio_set_level(BEEP_GPIO, 0);

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
    beep_init();

    Aht20 aht20(i2c_bus_handle, AHT20_ADDRESS_0);
    aht20.init();

    while (1) {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        aht20.update();
        aht20.print();
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
