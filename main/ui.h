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

private:
    static void slider_event_cb(lv_event_t *e);

public:
    DryerUI();
    void start_page();
    void main_page();
    void update(float temp, int16_t humi);
    void bind_indev(lv_indev_t *indev);
    void update_label_wifi(const char *text);
    void update_label_sensors(const char *text);
    void update_label_sntp(const char *text);


    int get_target_temp() const { return target_temp; }
    int get_duration() const { return duration; }
};

#ifdef __cplusplus
}
#endif



