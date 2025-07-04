/**
 * @file bsp_pwm.cc
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-07-04
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "bsp_pwm.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_log_level.h"
#include "esp_err.h"

static const char TAG[] = "pwm";


esp_err_t PWM::setupPWM(ledc_timer_t timer_num, uint32_t freq_hz,
    ledc_timer_bit_t duty_resolution, ledc_mode_t speed_mode,
    ledc_channel_t channel, uint32_t duty, gpio_num_t gpio) {

    esp_err_t ret = ESP_OK;

    this->timer_num_ = timer_num;
    this->freq_hz_ = freq_hz;
    this->duty_resolution_ = duty_resolution;
    this->speed_mode_ = speed_mode;
    this->channel_ = channel;
    this->duty_ = duty;
    this->gpio_ = gpio;

    ledc_timer_config_t timer_config = { };
    timer_config.speed_mode = this->speed_mode_;
    timer_config.duty_resolution = this->duty_resolution_;
    timer_config.timer_num = this->timer_num_;
    timer_config.freq_hz = this->freq_hz_;
    timer_config.clk_cfg = LEDC_AUTO_CLK;

    ledc_channel_config_t channel_config = { };
    channel_config.gpio_num = this->gpio_;
    channel_config.speed_mode = this->speed_mode_;
    channel_config.channel = this->channel_;
    channel_config.intr_type = LEDC_INTR_DISABLE;
    channel_config.timer_sel = this->timer_num_;
    channel_config.duty = this->duty_;

    ret = ledc_timer_config(&timer_config);
    ESP_RETURN_ON_ERROR(ret, TAG, "ledc timer config failed");
    ret = ledc_channel_config(&channel_config);
    ESP_RETURN_ON_ERROR(ret, TAG, "ledc channel config failed");

    ESP_LOGI(TAG, "pwm init done");

    return ret;
}

esp_err_t PWM::setDuty(uint8_t duty) {
    esp_err_t ret = ESP_OK;

    this->duty_to_percent_ = duty;
    this->duty_ = (1 << this->duty_resolution_) * duty / 100;
    ret = ledc_set_duty(this->speed_mode_, this->channel_, this->duty_);
    ESP_RETURN_ON_ERROR(ret, TAG, "set duty failed");
    ret = ledc_update_duty(this->speed_mode_, this->channel_);
    ESP_RETURN_ON_ERROR(ret, TAG, "update duty failed");

    return ret;
};

