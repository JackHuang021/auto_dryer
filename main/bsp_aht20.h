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

#ifndef __BSP_AHT20_H_
#define __BSP_AHT20_H_

#include <driver/i2c_master.h>
#include "aht20.h"


#ifdef __cplusplus
extern "C" {
#endif

class Aht20 {
private:
    i2c_master_bus_handle_t i2c_bus_handle;
    aht20_dev_handle_t aht20_handle;
    uint8_t dev_addr;
    int16_t humi;
    int16_t temp;

public:
    Aht20(i2c_master_bus_handle_t i2c_bus, uint8_t addr);
    esp_err_t init(void);
    esp_err_t update(void);
    void print(void);
    int16_t get_humi(void);
    int16_t get_temp(void);

protected:
};

#endif // I2C_DEVICE_H


#ifdef __cplusplus
}
#endif
