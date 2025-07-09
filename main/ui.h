/**
 * @file ui.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-07-02
 * 
 * @copyright Copyright (c) 2025
 * 
 */

 #pragma once

#include <stdint.h>
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

enum setup_step {
    SETUP_SENSORS,
    SETUP_WIFI,
    SETUP_SMARTCONFIG,
    SETUP_TIME,
};

enum ui_page {
    START_PAGE,
    MAIN_PAGE,
    PAGE_NUMS,
};

class DryerUI {
private:
    lv_obj_t *label_sensors_ = NULL;
    lv_obj_t *label_wifi_ = NULL;
    lv_obj_t *label_sntp_ = NULL;

    lv_obj_t *label_time_ = NULL;
    lv_obj_t *label_temp_ = NULL;
    lv_obj_t *label_humi_ = NULL;
    lv_obj_t *label_target_value_ = NULL;
    lv_obj_t *label_duration_value_ = NULL;
    lv_obj_t *slider_target_ = NULL;
    lv_obj_t *slider_duration_ = NULL;
    lv_group_t *indev_group_ = NULL;

    int target_temp = 40;
    int duration = 10;
    ui_page current_page_ = START_PAGE;

private:
    static void slider_event_cb(lv_event_t *e);

public:
    DryerUI();
    void start_page();
    void main_page();
    void update_main_page(float temp, int16_t humi);
    void bind_indev(lv_indev_t *indev);

    int get_target_temp() const { return target_temp; }
    int get_duration() const { return duration; }

    void update_start_page(setup_step step, bool status);
};

#ifdef __cplusplus
}
#endif



