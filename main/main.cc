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
#include "freertos/event_groups.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_check.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif_types.h"

#include "config.h"
#include "bsp_aht20.h"
#include "Buzzer.h"
#include "iot_button.h"
#include "iot_knob.h"
#include "button_gpio.h"
#include "lvgl.h"
#include "bsp_lcd.h"
#include "ui.h"
#include "bsp_pwm.h"
#include "dryer.h"
#include "smartconfig.h"
#include "sntp.h"

static const char *TAG = "main";

static Dryer dryer;
static SyncTime synctime;
static WiFiConnect wifi;

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

    dryer.init();
    wifi.init();
    EventBits_t bits = xEventGroupWaitBits(wifi.wifi_event_group_,
                                           WIFI_CONNECTED | SMARTCONFIG_START,
                                           true, false, pdMS_TO_TICKS(20000));
    if (bits & WIFI_CONNECTED) {
        dryer.ui_.update_start_page(SETUP_WIFI, true);
        synctime.sync();
    }
    else
        dryer.ui_.update_start_page(SETUP_WIFI, false);

    if (bits & SMARTCONFIG_START)
        dryer.ui_.update_start_page(SETUP_SMARTCONFIG, true);

    vTaskDelay(pdMS_TO_TICKS(1000));
    dryer.ui_.main_page();
    dryer.ui_.bind_indev(Dryer::indev_);
}
