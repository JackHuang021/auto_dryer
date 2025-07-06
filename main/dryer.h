/**
 * @file heater.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-07-04
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "Buzzer.h"
#include "bsp_aht20.h"
#include "bsp_pwm.h"
#include "ui.h"
#include "iot_knob.h"
#include "iot_button.h"
#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

class Dryer {

public:
    static Buzzer buz_;
    static Aht20 aht20_;
    static PWM ptc_pwm_;
    static DryerUI ui_;
    static button_handle_t btn_handle_;
    static knob_handle_t knob_handle_;
    static i2c_master_bus_handle_t i2c_bus_handle_;
    static lv_indev_t *indev_;
    static bool inited_;
    static bool running_;
    static TaskHandle_t main_task_;
    static TaskHandle_t ui_task_;

public:
    float temp_ = 0;
    int16_t humi_ = 0;
    uint16_t target_temp_ = 30;
    uint16_t duration_ = 0;
    uint16_t remain_ = 0;

private:


public:
    esp_err_t init(void);
};


#ifdef __cplusplus
}
#endif
