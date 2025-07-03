/**
 * @file bsp_pwm.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-07-04
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <stdint.h>
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_check.h"

#ifdef __cplusplus
extern "C" {
#endif

class PWM {
private:
    ledc_timer_t timer_num_;
    uint32_t freq_hz_;
    ledc_timer_bit_t duty_resolution_;
    ledc_mode_t speed_mode_;
    ledc_channel_t channel_;
    uint32_t duty_;
    uint8_t duty_to_percent_;
    gpio_num_t gpio_;

public:
    esp_err_t setupPWM(ledc_timer_t timer_num, uint32_t freq_hz,
            ledc_timer_bit_t duty_resolution, ledc_mode_t speed_mode,
            ledc_channel_t channel, uint32_t duty, gpio_num_t gpio);

    esp_err_t setDuty(uint8_t duty);
};


#ifdef __cplusplus
}
#endif
