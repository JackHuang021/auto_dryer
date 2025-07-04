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
#include "config.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_err.h"

#include "lvgl.h"
#include <stdio.h>

LV_FONT_DECLARE(font_awesome);

static const char TAG[] = "UI";

DryerUI::DryerUI() {
    // 构造函数可留空
}

void DryerUI::slider_event_cb(lv_event_t *e)
{
    DryerUI *ui = reinterpret_cast<DryerUI *>(lv_event_get_user_data(e));
    lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
    int raw_value = lv_slider_get_value(slider);

    if (slider == ui->slider_target) {
        ui->target_temp = raw_value;
        lv_label_set_text_fmt(ui->label_target_value, "%d°C", raw_value);
    } else if (slider == ui->slider_duration) {
        const int step = 10;
        int last_value = ui->duration;
        int value = 0;

        if (raw_value > last_value)
            value = last_value + step;
        else
            value = last_value - step;

        if (value < lv_slider_get_min_value(slider))
            value = lv_slider_get_min_value(slider);
        if (value > lv_slider_get_max_value(slider))
            value = lv_slider_get_max_value(slider);

        ui->duration = value;
        lv_label_set_text_fmt(ui->label_duration_value, "%d min", value);
        lv_slider_set_value(slider, value, LV_ANIM_OFF);
    }
}

void DryerUI::init() {
    int y_pos = 0;
    lv_obj_t *scr = lv_scr_act();

    lvgl_port_lock(0);

    lv_obj_set_style_bg_color(scr, lv_color_hex(0x101010), 0);

    // 时间
    y_pos += 5;
    label_time = lv_label_create(scr);
    lv_label_set_text(label_time, "--:--");
    lv_obj_set_style_text_color(label_time, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_time, &lv_font_montserrat_20, 0);
    lv_obj_align(label_time, LV_ALIGN_TOP_MID, 0, y_pos);

    // 当前温度
    y_pos += 40;
    lv_obj_t *icon_temp = lv_label_create(scr);
    lv_obj_set_style_text_font(icon_temp, &font_awesome, 0);
    lv_label_set_text(icon_temp, "\xEF\x8B\x88");
    lv_obj_set_style_text_color(icon_temp, lv_color_hex(0xc23616), 0);
    lv_obj_align(icon_temp, LV_ALIGN_TOP_LEFT, 10, y_pos);

    label_temp = lv_label_create(scr);
    lv_obj_set_style_text_color(label_temp, lv_color_hex(0xc23616), 0);
    lv_obj_align_to(label_temp, icon_temp, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    // 当前湿度
    lv_obj_t *icon_humi = lv_label_create(scr);
    lv_obj_set_style_text_font(icon_humi, &font_awesome, 0);
    lv_label_set_text(icon_humi, "\xEF\x81\x83");
    lv_obj_set_style_text_color(icon_humi, lv_color_hex(0x3498db), 0);
    lv_obj_align(icon_humi, LV_ALIGN_TOP_RIGHT, -85, y_pos);

    label_humi = lv_label_create(scr);
    lv_obj_set_style_text_color(label_humi, lv_color_hex(0x3498db), 0);
    lv_obj_align_to(label_humi, icon_humi, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    // 目标温度滑块
    y_pos += 55;
    lv_obj_t *icon_target = lv_label_create(scr);
    lv_obj_set_style_text_font(icon_target, &font_awesome, 0);
    lv_label_set_text(icon_target, "\xEF\x81\xAD");
    lv_obj_set_style_text_color(icon_target, lv_color_hex(0xc23616), 0);
    lv_obj_align(icon_target, LV_ALIGN_TOP_LEFT, 10, y_pos);

    label_target_value = lv_label_create(scr);
    lv_obj_set_style_text_color(label_target_value, lv_color_hex(0xc23616), 0);
    lv_obj_align_to(label_target_value, icon_target,
                    LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    y_pos += 35;
    slider_target = lv_slider_create(scr);
    lv_slider_set_range(slider_target, MIN_HEAT_TEMP, MAX_HEAT_TEMP);
    lv_slider_set_value(slider_target, target_temp, LV_ANIM_OFF);
    lv_obj_set_size(slider_target, 200, 10);
    lv_obj_align(slider_target, LV_ALIGN_TOP_LEFT, 10, y_pos);
    lv_obj_add_event_cb(slider_target, slider_event_cb,
                        LV_EVENT_VALUE_CHANGED, this);

    // 加热时间滑块
    y_pos += 30;
    lv_obj_t *icon_duration = lv_label_create(scr);
    lv_obj_set_style_text_font(icon_duration, &font_awesome, 0);
    lv_label_set_text(icon_duration, "\xEF\x80\x97");
    lv_obj_set_style_text_color(icon_duration, lv_color_hex(0x44bd32), 0);
    lv_obj_align(icon_duration, LV_ALIGN_TOP_LEFT, 10, y_pos);

    label_duration_value = lv_label_create(scr);
    lv_obj_set_style_text_color(label_duration_value, lv_color_hex(0x44bd32), 0);
    lv_obj_align_to(label_duration_value, icon_duration,
                    LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    y_pos += 35;
    slider_duration = lv_slider_create(scr);
    lv_slider_set_range(slider_duration, MIN_HEAT_TIME_MINS, MAX_HEAT_TIME_MINS);
    lv_slider_set_value(slider_duration, duration, LV_ANIM_OFF);
    
    lv_obj_set_size(slider_duration, 200, 10);
    lv_obj_align(slider_duration, LV_ALIGN_TOP_LEFT, 10, y_pos);
    lv_obj_add_event_cb(slider_duration, slider_event_cb,
                        LV_EVENT_VALUE_CHANGED, this);

    group = lv_group_create();
    lv_group_add_obj(group, slider_target);
    lv_group_add_obj(group, slider_duration);

    lvgl_port_unlock();

    ESP_LOGI(TAG, "ui init done");
}

void DryerUI::bind_indev(lv_indev_t *indev)
{
    lvgl_port_lock(0);
    lv_indev_set_group(indev, group);
    lvgl_port_unlock();
}

void DryerUI::update(float temp, int16_t humi)
{
    char buf[16] = {0};

    lvgl_port_lock(0);
    lv_label_set_text_fmt(label_time, "18:32");

    snprintf(buf, sizeof(buf), "%.1f°C", temp);
    lv_label_set_text(label_temp, buf);

    snprintf(buf, sizeof(buf), "%d%%", humi);
    lv_label_set_text(label_humi, buf);

    lv_label_set_text_fmt(label_target_value, "%d°C", target_temp);
    lv_label_set_text_fmt(label_duration_value, "%d min", duration);
    lvgl_port_unlock();
}