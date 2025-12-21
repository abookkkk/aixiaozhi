#pragma once

#include "esp_lvgl_port.h"
#include "xiaozhi_lcd.h"
#include "esp_log.h"
#include "xiaozhi_wifi.h"
#include "stdint.h"

void xiaozhi_lvgl_init(void);

void xiaozhi_lvgl_helloworld(void);

void xiaozhi_lvgl_qrcode_show(char *data);

void xiaozhi_lvgl_del_qrcode(void);

