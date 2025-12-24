#pragma once

#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "cJSON.h"
#include "esp_log.h"
#include "esp_app_desc.h"
#include "esp_wifi.h"
#include "esp_tls.h"
#include "stdlib.h"
#include "string.h"
#include "common/common_types.h"
#include "xiaozhi_lvgl.h"

void xiaozhi_active_request(void);