/**
 * @file bsp_aht20.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-06-28
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <driver/i2c_master.h>
#include "aht20.h"


#ifdef __cplusplus
extern "C" {
#endif

class Aht20 {
private:
    i2c_master_bus_handle_t i2c_bus_handle = NULL;
    aht20_dev_handle_t aht20_handle = NULL;
    uint8_t dev_addr = 0;
    int16_t humi = 0;
    int16_t temp = 0;

public:
    esp_err_t init(i2c_master_bus_handle_t i2c_bus, uint8_t addr);
    esp_err_t update(void);
    void print(void);
    int16_t get_humi(void);
    int16_t get_temp(void);

protected:
};

#ifdef __cplusplus
}
#endif
