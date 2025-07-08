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

#include <stdio.h>
#include <assert.h>

#include "bsp_lcd.h"
#include "ui.h"
#include "esp_lvgl_port.h"
#include "config.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_macros.h"

#include "lvgl.h"


LV_FONT_DECLARE(font_awesome);

static const char TAG[] = "UI";

DryerUI::DryerUI() { }

void DryerUI::slider_event_cb(lv_event_t *e)
{
    DryerUI *ui = reinterpret_cast<DryerUI *>(lv_event_get_user_data(e));
    lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
    int raw_value = lv_slider_get_value(slider);

    if (slider == ui->slider_target_) {
        ui->target_temp = raw_value;
        lv_label_set_text_fmt(ui->label_target_value_, "%d°C", raw_value);
    } else if (slider == ui->slider_duration_) {
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
        lv_label_set_text_fmt(ui->label_duration_value_, "%d min", value);
        lv_slider_set_value(slider, value, LV_ANIM_OFF);
    }
}

void DryerUI::start_page() {
    int y_pos = 0;
    lv_obj_t *scr = lv_scr_act();

    lvgl_port_lock(0);

    // set background color
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x101010), 0);

    // sensor status
    y_pos += 35;
    label_sensors_ = lv_obj_create(scr);
    lv_obj_set_style_text_color(label_sensors_, lv_color_hex(0x3498db), 0);
    lv_label_set_text(label_sensors_, LV_SYMBOL_LIST "Init sensors...");
    lv_obj_align(label_sensors_, LV_ALIGN_TOP_LEFT, 10, y_pos);


    // connect to wifi
    y_pos += 35;
    label_wifi_ = lv_obj_create(scr);
    lv_obj_set_style_text_color(label_wifi_, lv_color_hex(0x3498db), 0);
    lv_label_set_text(label_wifi_, "");
    lv_obj_align(label_wifi_, LV_ALIGN_TOP_LEFT, 10, y_pos);

    // sync time
    y_pos += 35;
    label_sntp_ = lv_obj_create(scr);
    lv_obj_set_style_text_color(label_sntp_, lv_color_hex(0x3498db), 0);
    lv_label_set_text(label_sntp_, "");
    lv_obj_align(label_sntp_, LV_ALIGN_TOP_LEFT, 10, y_pos);

    lvgl_port_unlock();
}


void DryerUI::main_page() {
    int y_pos = 0;
    lv_obj_t *scr = lv_scr_act();

    lvgl_port_lock(0);

    lv_obj_set_style_bg_color(scr, lv_color_hex(0x101010), 0);

    // 时间
    y_pos += 5;
    label_time_ = lv_label_create(scr);
    lv_label_set_text(label_time_, "--:--");
    lv_obj_set_style_text_color(label_time_, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_time_, &lv_font_montserrat_20, 0);
    lv_obj_align(label_time_, LV_ALIGN_TOP_MID, 0, y_pos);

    // 当前温度
    y_pos += 40;
    lv_obj_t *icon_temp = lv_label_create(scr);
    lv_obj_set_style_text_font(icon_temp, &font_awesome, 0);
    lv_label_set_text(icon_temp, "\xEF\x8B\x88");
    lv_obj_set_style_text_color(icon_temp, lv_color_hex(0xc23616), 0);
    lv_obj_align(icon_temp, LV_ALIGN_TOP_LEFT, 10, y_pos);

    label_temp_ = lv_label_create(scr);
    lv_obj_set_style_text_color(label_temp_, lv_color_hex(0xc23616), 0);
    lv_obj_align_to(label_temp_, icon_temp, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    // 当前湿度
    lv_obj_t *icon_humi = lv_label_create(scr);
    lv_obj_set_style_text_font(icon_humi, &font_awesome, 0);
    lv_label_set_text(icon_humi, "\xEF\x81\x83");
    lv_obj_set_style_text_color(icon_humi, lv_color_hex(0x3498db), 0);
    lv_obj_align(icon_humi, LV_ALIGN_TOP_RIGHT, -85, y_pos);

    label_humi_ = lv_label_create(scr);
    lv_obj_set_style_text_color(label_humi_, lv_color_hex(0x3498db), 0);
    lv_obj_align_to(label_humi_, icon_humi, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    // 目标温度滑块
    y_pos += 55;
    lv_obj_t *icon_target = lv_label_create(scr);
    lv_obj_set_style_text_font(icon_target, &font_awesome, 0);
    lv_label_set_text(icon_target, "\xEF\x81\xAD");
    lv_obj_set_style_text_color(icon_target, lv_color_hex(0xc23616), 0);
    lv_obj_align(icon_target, LV_ALIGN_TOP_LEFT, 10, y_pos);

    label_target_value_ = lv_label_create(scr);
    lv_obj_set_style_text_color(label_target_value_, lv_color_hex(0xc23616), 0);
    lv_obj_align_to(label_target_value_, icon_target,
                    LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    y_pos += 35;
    slider_target_ = lv_slider_create(scr);
    lv_slider_set_range(slider_target_, MIN_HEAT_TEMP, MAX_HEAT_TEMP);
    lv_slider_set_value(slider_target_, target_temp, LV_ANIM_OFF);
    lv_obj_set_size(slider_target_, 200, 10);
    lv_obj_align(slider_target_, LV_ALIGN_TOP_LEFT, 10, y_pos);
    lv_obj_add_event_cb(slider_target_, slider_event_cb,
                        LV_EVENT_VALUE_CHANGED, this);

    // 加热时间滑块
    y_pos += 30;
    lv_obj_t *icon_duration = lv_label_create(scr);
    lv_obj_set_style_text_font(icon_duration, &font_awesome, 0);
    lv_label_set_text(icon_duration, "\xEF\x80\x97");
    lv_obj_set_style_text_color(icon_duration, lv_color_hex(0x44bd32), 0);
    lv_obj_align(icon_duration, LV_ALIGN_TOP_LEFT, 10, y_pos);

    label_duration_value_ = lv_label_create(scr);
    lv_obj_set_style_text_color(label_duration_value_, lv_color_hex(0x44bd32), 0);
    lv_obj_align_to(label_duration_value_, icon_duration,
                    LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    y_pos += 35;
    slider_duration_ = lv_slider_create(scr);
    lv_slider_set_range(slider_duration_, MIN_HEAT_TIME_MINS, MAX_HEAT_TIME_MINS);
    lv_slider_set_value(slider_duration_, duration, LV_ANIM_OFF);
    
    lv_obj_set_size(slider_duration_, 200, 10);
    lv_obj_align(slider_duration_, LV_ALIGN_TOP_LEFT, 10, y_pos);
    lv_obj_add_event_cb(slider_duration_, slider_event_cb,
                        LV_EVENT_VALUE_CHANGED, this);

    indev_group_ = lv_group_create();
    lv_group_add_obj(indev_group_, slider_target_);
    lv_group_add_obj(indev_group_, slider_duration_);

    lvgl_port_unlock();

    ESP_LOGI(TAG, "ui init done");
}

void DryerUI::bind_indev(lv_indev_t *indev)
{
    lvgl_port_lock(0);
    lv_indev_set_group(indev, indev_group_);
    lvgl_port_unlock();
}

void DryerUI::update_label_wifi(const char *text)
{
    assert(label_wifi_ != NULL);

    lvgl_port_lock(0);
    lv_label_set_text(label_wifi_, text);
    lvgl_port_unlock();
}

void DryerUI::update_label_sensors(const char *text)
{
    assert(label_sensors_ != NULL);

    lvgl_port_lock(0);
    lv_label_set_text(label_sensors_, text);
    lvgl_port_unlock();
}

void DryerUI::update_label_sntp(const char *text)
{
    assert(label_sntp_ != NULL);

    lvgl_port_lock(0);
    lv_label_set_text(label_sntp_, text);
    lvgl_port_unlock();
}

void DryerUI::update(float temp, int16_t humi)
{
    char buf[16] = {0};

    lvgl_port_lock(0);
    lv_label_set_text_fmt(label_time_, "18:32");

    snprintf(buf, sizeof(buf), "%.1f°C", temp);
    lv_label_set_text(label_temp_, buf);

    snprintf(buf, sizeof(buf), "%d%%", humi);
    lv_label_set_text(label_humi_, buf);

    lv_label_set_text_fmt(label_target_value_, "%d°C", target_temp);
    lv_label_set_text_fmt(label_duration_value_, "%d min", duration);
    lvgl_port_unlock();
}