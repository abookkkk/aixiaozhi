#ifndef __XIAOZHI_WIFI_H__
#define __XIAOZHI_WIFI_H__
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_ble.h"

#include "qrcode.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "xiaozhi_lvgl.h"
void xiaozhi_wifi_init(void);

void xiaozhi_wifi_restart(void);

#endif /* __XIAOZHI_WIFI_H__ */
