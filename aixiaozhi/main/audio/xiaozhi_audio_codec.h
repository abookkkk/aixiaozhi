#pragma once

#include "esp_opus_enc.h"
#include "stddef.h"
#include "common/common_types.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/idf_additions.h"
#include "esp_log.h"

void xiaozhi_audio_codec_init(void);