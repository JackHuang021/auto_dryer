/**
 * @file heater.cc
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-07-04
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "dryer.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "button_gpio.h"
#include "bsp_lcd.h"
#include "lvgl.h"
#include "config.h"

static const char TAG[] = "dryer";

/* static member init */
i2c_master_bus_handle_t Dryer::i2c_bus_handle_ = NULL;
button_handle_t Dryer::btn_handle_ = NULL;
knob_handle_t Dryer::knob_handle_ = NULL;
lv_indev_t * Dryer::indev_ = NULL;
TaskHandle_t Dryer::main_task_ = NULL;
TaskHandle_t Dryer::ui_task_ = NULL;
bool Dryer::inited_ = false;
bool Dryer::running_ = false;
Buzzer Dryer::buz_;
DryerUI Dryer::ui_;
PWM Dryer::ptc_pwm_;
Aht20 Dryer::aht20_;
static bool btn_state = false;

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

    ret = i2c_new_master_bus(&i2c_bus_config, &Dryer::i2c_bus_handle_);
    ESP_RETURN_ON_ERROR(ret, TAG, "failed to init i2c bus: %s",
                        esp_err_to_name(ret));

    ESP_LOGI(TAG, "i2c bus init done.");

    return ret;
}

static void button_press_down_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "BTN: BUTTON_PRESS_UP");
    if (iot_button_get_ticks_time(Dryer::btn_handle_) <
            CONFIG_BUTTON_LONG_PRESS_TIME_MS) {
        btn_state = true;
        Dryer::buz_.Beep( {1000, 200, 0.1f} );
    }
}

static void button_long_press_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "BTN: BUTTON_LONG_PRESS_START");
    Dryer::running_ = !Dryer::running_;
    Dryer::buz_.Beep( {800, 500, 0.1f} );
}

static void knob_right_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "KONB: KONB_RIGHT, count_value:%d",
             iot_knob_get_count_value((button_handle_t)arg));
    Dryer::buz_.Beep( {500, 100, 0.1f} );
}

static void knob_left_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "KONB: KONB_LEFT, count_value:%d",
             iot_knob_get_count_value((button_handle_t)arg));
    Dryer::buz_.Beep( {500, 100, 0.1f} );
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

    ret = iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &Dryer::btn_handle_);
    ESP_RETURN_ON_ERROR(ret, TAG, "init button failed");
    ret = iot_button_register_cb(Dryer::btn_handle_, BUTTON_PRESS_UP, NULL,
                                 button_press_down_cb, NULL);
    ESP_RETURN_ON_ERROR(ret, TAG, "failed to register BUTTON_PRESS_DOWN cb");
    ret = iot_button_register_cb(Dryer::btn_handle_, BUTTON_LONG_PRESS_START,
                                 NULL, button_long_press_cb, NULL);
    ESP_RETURN_ON_ERROR(ret, TAG, "failed to register BUTTON_LONG_PRESS_START cb");

    knob_config_t knob_config = {
        .default_direction = 0,
        .gpio_encoder_a = EC11_A_GPIO,
        .gpio_encoder_b = EC11_B_GPIO,
        .enable_power_save = false,
    };

    Dryer::knob_handle_ = iot_knob_create(&knob_config);
    if (NULL == Dryer::knob_handle_) {
        ESP_LOGE(TAG, "knob create failed");
    }

    ret = iot_knob_register_cb(Dryer::knob_handle_, KNOB_LEFT,
                               knob_left_cb, NULL);
    ESP_RETURN_ON_ERROR(ret, TAG, "failed to register KNOB_LEFT cb");
    ret = iot_knob_register_cb(Dryer::knob_handle_, KNOB_RIGHT, knob_right_cb, NULL);
    ESP_RETURN_ON_ERROR(ret, TAG, "failed to register KNOB_RIGHT cb");

    ESP_LOGI(TAG, "ec11 init done");

    return ret;
}

static void lv_indev_read_cb(lv_indev_t * indev, lv_indev_data_t * data)
{
    int cnt_value = iot_knob_get_count_value(Dryer::knob_handle_);

    if (cnt_value != 0) {
        data->enc_diff = cnt_value;
        iot_knob_clear_count_value(Dryer::knob_handle_);
        data->state = LV_INDEV_STATE_RELEASED;
    } else {
        data->state = btn_state ?
                    LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
        data->enc_diff = 0;
        btn_state = false;
        iot_knob_clear_count_value(Dryer::knob_handle_);
    }
}

static esp_err_t lvgl_indev_init(void)
{
    esp_err_t ret = ESP_OK;

    Dryer::indev_ = lv_indev_create();
    ESP_RETURN_ON_FALSE(Dryer::indev_, ESP_ERR_NO_MEM, TAG, "not enough memory for lv_indev_t");
    lv_indev_set_type(Dryer::indev_ , LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(Dryer::indev_ , lv_indev_read_cb);
    lv_indev_set_display(Dryer::indev_ , get_lvgl_display());

    ESP_LOGI(TAG, "register lvgl input device success");

    return ret;
}

static void ui_update_task(void *args)
{
    Dryer *dryer = static_cast<Dryer *>(args);

    while (1) {
        Dryer::ui_.update(dryer->temp_, dryer->humi_);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void main_update_task(void *args)
{
    Dryer *dryer = static_cast<Dryer *>(args);

    while (1) {
        Dryer::aht20_.update();
        dryer->temp_ = static_cast<float>(Dryer::aht20_.get_temp()) / 100;
        dryer->humi_ = Dryer::aht20_.get_humi() / 100;

        if (!Dryer::running_) {
            Dryer::ptc_pwm_.setDuty(0);
            continue;
        }

        if (Dryer::running_)
            Dryer::ptc_pwm_.setDuty(100);
    }
}

esp_err_t Dryer::init(void)
{
    esp_err_t ret = ESP_OK;

    /* init i2c bus & aht20 */
    ret = i2c_init();
    ret = aht20_.init(i2c_bus_handle_, AHT20_ADDRESS_0);

    /* init beep */
    ret = Dryer::buz_.Init(BEEP_GPIO);

    /* init spi lcd & lvgl */
    ret = lcd_init();
    ret = lvgl_init();
    ui_.start_page();

    /* init ec11 */
    ret = ec11_init();
    ret = lvgl_indev_init();

    Dryer::ptc_pwm_.setupPWM(LEDC_TIMER_1, 200, LEDC_TIMER_13_BIT,
                             LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1,
                             0, PTC_GPIO);
    Dryer::ptc_pwm_.setDuty(0);

    if (ESP_OK == ret)
        Dryer::ui_.update_start_page(SETUP_SENSORS, true);
    else {
        Dryer::ui_.update_start_page(SETUP_SENSORS, false);
        ESP_LOGE(TAG, "init sensors failed, exit");
        return ret;
    }

    Dryer::buz_.Play({{800, 200, 0.01f}, {500, 200, 0.01f}});

    Dryer::inited_ = true;

    BaseType_t res = pdPASS;
    res = xTaskCreate(ui_update_task, "ui_task", 4096, this,
                      configMAX_PRIORITIES - 3, &Dryer::ui_task_);
    if (pdPASS != res) {
        ESP_LOGE(TAG, "failed to create ui task");
        ret = ESP_FAIL;
    }

    res = xTaskCreate(main_update_task, "main_task", 2048, this,
                      configMAX_PRIORITIES - 2, &Dryer::main_task_);
    if (pdPASS != res) {
    ESP_LOGE(TAG, "failed to create main task");
    ret = ESP_FAIL;
    }

    return ret;
}

