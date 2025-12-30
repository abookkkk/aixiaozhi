#pragma once

#include "esp_lvgl_port.h"
#include "xiaozhi_lcd.h"
#include "esp_log.h"
#include "xiaozhi_wifi.h"
#include "stdint.h"
#include "common/common_types.h"
#include "font_puhui.h"
#include "font_emoji.h"
extern const lv_font_t font_puhui_14_1;
extern const lv_font_t font_puhui_16_4;
extern const lv_font_t font_puhui_20_4;



void xiaozhi_lvgl_init(void);

void xiaozhi_lvgl_helloworld(void);

void xiaozhi_lvgl_qrcode_show(char *data);

void xiaozhi_lvgl_del_qrcode(void);

void xiaozhi_lvgl_title_label_update(char *text);

void xiaozhi_lvgl_emoji_label_update(char *emoji);

void xiaozhi_lvgl_center_text_update(char *text);