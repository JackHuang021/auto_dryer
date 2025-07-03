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

class HeaterUI {
public:
    HeaterUI();
    void init();
    void update(float temp, int16_t humi);
    void bind_indev(lv_indev_t *indev);

    int get_target_temp() const { return target_temp; }
    int get_duration() const { return duration; }

private:
    lv_obj_t *label_time;
    lv_obj_t *label_temp;
    lv_obj_t *label_humi;
    lv_obj_t *label_target_value;
    lv_obj_t *label_duration_value;
    lv_obj_t *slider_target;
    lv_obj_t *slider_duration;
    lv_group_t *group;

    int target_temp = 40;
    int duration = 10;

    static void slider_event_cb(lv_event_t *e);
};

#ifdef __cplusplus
}
#endif



