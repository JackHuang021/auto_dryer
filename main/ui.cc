/**
 * @file ui.cc
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-07-02
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "bsp_lcd.h"

#include "ui.h"
#include "esp_lvgl_port.h"



#include "lvgl.h"
#include <stdio.h>

HeaterUI::HeaterUI() {
    // 构造函数可留空
}

void HeaterUI::slider_event_cb(lv_event_t *e)
{
    HeaterUI *ui = reinterpret_cast<HeaterUI *>(lv_event_get_user_data(e));
    lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
    int value = lv_slider_get_value(slider);

    if (slider == ui->slider_target) {
        ui->target_temp = value;
        lv_label_set_text_fmt(ui->label_target_value, "%d°C", value);
    } else if (slider == ui->slider_duration) {
        ui->duration_min = value;
        lv_label_set_text_fmt(ui->label_duration_value, "%d min", value);
    }
}

void HeaterUI::init() {
    int y_pos = 0;
    lv_obj_t *scr = lv_scr_act();

    lvgl_port_lock(0);

    lv_obj_set_style_bg_color(scr, lv_color_hex(0x101010), 0);

    // 时间
    y_pos += 5;
    label_time = lv_label_create(scr);
    lv_label_set_text(label_time, "---- -- -- --:--");
    lv_obj_set_style_text_color(label_time, lv_color_white(), 0);
    lv_obj_align(label_time, LV_ALIGN_TOP_MID, 0, y_pos);

    // 当前温度
    y_pos += 45;
    lv_obj_t *icon_temp = lv_label_create(scr);
    lv_label_set_text(icon_temp, LV_SYMBOL_CHARGE);
    lv_obj_set_style_text_color(icon_temp, lv_color_hex(0xFF5555), 0);
    lv_obj_align(icon_temp, LV_ALIGN_TOP_LEFT, 10, y_pos);

    label_temp = lv_label_create(scr);
    lv_obj_set_style_text_color(label_temp, lv_color_hex(0xFF5555), 0);
    lv_obj_align_to(label_temp, icon_temp, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    // 当前湿度
    lv_obj_t *icon_humi = lv_label_create(scr);
    lv_label_set_text(icon_humi, LV_SYMBOL_LOOP);
    lv_obj_set_style_text_color(icon_humi, lv_color_hex(0x55AAFF), 0);
    lv_obj_align(icon_humi, LV_ALIGN_TOP_RIGHT, -80, y_pos);

    label_humi = lv_label_create(scr);
    lv_obj_set_style_text_color(label_humi, lv_color_hex(0x55AAFF), 0);
    lv_obj_align_to(label_humi, icon_humi, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    // 目标温度滑块
    y_pos += 35;
    lv_obj_t *icon_target = lv_label_create(scr);
    lv_label_set_text(icon_target, LV_SYMBOL_SETTINGS);
    lv_obj_set_style_text_color(icon_target, lv_color_hex(0xFFA500), 0);
    lv_obj_align(icon_target, LV_ALIGN_TOP_LEFT, 10, y_pos);

    label_target_value = lv_label_create(scr);
    lv_obj_set_style_text_color(label_target_value, lv_color_hex(0xAAAAFF), 0);
    lv_obj_align_to(label_target_value, icon_target, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    y_pos += 35;
    slider_target = lv_slider_create(scr);
    lv_slider_set_range(slider_target, 20, 60);
    lv_slider_set_value(slider_target, target_temp, LV_ANIM_OFF);
    lv_obj_set_size(slider_target, 200, 10);
    lv_obj_align(slider_target, LV_ALIGN_TOP_LEFT, 10, y_pos);
    lv_obj_add_event_cb(slider_target, slider_event_cb, LV_EVENT_VALUE_CHANGED, this);

    // 加热时间滑块
    y_pos += 35;
    lv_obj_t *icon_duration = lv_label_create(scr);
    lv_label_set_text(icon_duration, LV_SYMBOL_PLAY);
    lv_obj_set_style_text_color(icon_duration, lv_color_hex(0x00D3A3), 0);
    lv_obj_align(icon_duration, LV_ALIGN_TOP_LEFT, 10, y_pos);

    label_duration_value = lv_label_create(scr);
    lv_obj_set_style_text_color(label_duration_value, lv_color_hex(0xAAAAFF), 0);
    lv_obj_align_to(label_duration_value, icon_duration, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    y_pos += 35;
    slider_duration = lv_slider_create(scr);
    lv_slider_set_range(slider_duration, 1, 60);
    lv_slider_set_value(slider_duration, duration_min, LV_ANIM_OFF);
    lv_obj_set_size(slider_duration, 200, 10);
    lv_obj_align(slider_duration, LV_ALIGN_TOP_LEFT, 10, y_pos);
    lv_obj_add_event_cb(slider_duration, slider_event_cb, LV_EVENT_VALUE_CHANGED, this);

    // 剩余时间
    // lv_obj_t *icon_remain = lv_label_create(scr);
    // lv_label_set_text(icon_remain, LV_SYMBOL_BELL);
    // lv_obj_set_style_text_color(icon_remain, lv_color_hex(0x00D3A3), 0);
    // lv_obj_align(icon_remain, LV_ALIGN_BOTTOM_LEFT, 10, -5);

    // label_remain = lv_label_create(scr);
    // lv_obj_set_style_text_color(label_remain, lv_color_hex(0x55FF55), 0);
    // lv_obj_align_to(label_remain, label_remain, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    lvgl_port_unlock();
}

void HeaterUI::update(float temp, float humi, int remain_m, int remain_s)
{
    char buf[16] = {0};

    lvgl_port_lock(0);
    lv_label_set_text_fmt(label_time, "2025-07-02 18:32");

    snprintf(buf, sizeof(buf), "%.1f°C", temp);
    lv_label_set_text(label_temp, buf);

    snprintf(buf, sizeof(buf), "%.1f°C", humi);
    lv_label_set_text(label_humi, buf);

    lv_label_set_text_fmt(label_target_value, "%d°C", target_temp);
    lv_label_set_text_fmt(label_duration_value, "%d min", duration_min);
    // lv_label_set_text_fmt(label_remain, "%02d:%02d", remain_m, remain_s);
    lvgl_port_unlock();
}