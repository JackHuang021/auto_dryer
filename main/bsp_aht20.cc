/**
 * @file bsp_aht20.cc
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-06-28
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "bsp_aht20.h"
#include "esp_log.h"
#include "config.h"

static const char *TAG = "aht20";

esp_err_t Aht20::init(i2c_master_bus_handle_t i2c_bus, uint8_t addr)
{
    esp_err_t ret = ESP_OK;

    this->i2c_bus_handle = i2c_bus;
    this->dev_addr = addr;

    i2c_aht20_config_t aht20_i2c_config = {};
    aht20_i2c_config.i2c_config.device_address = this->dev_addr;
    aht20_i2c_config.i2c_config.scl_speed_hz = DEFAULT_I2C_CLOCK;
    aht20_i2c_config.i2c_timeout = 1000;

    ret = aht20_new_sensor(this->i2c_bus_handle,
                           &aht20_i2c_config, &aht20_handle);
    ESP_RETURN_ON_ERROR(ret, TAG, "failed to init aht20");

    ESP_LOGI(TAG, "aht20 init done.");

    return ret;
}

esp_err_t Aht20::update(void)
{
    return aht20_read_i16(aht20_handle, &temp, &humi);
}

void Aht20::print(void)
{
    ESP_LOGI(TAG, "temp %i, humi %i", temp, humi);
}

int16_t Aht20::get_humi(void)
{
    return humi;
}

int16_t Aht20::get_temp(void)
{
    return temp;
}
