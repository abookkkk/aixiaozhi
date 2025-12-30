#pragma once
#include "driver/i2c_master.h"
#include "driver/i2s_std.h"
#include "driver/i2s_tdm.h"
#include "esp_codec_dev_defaults.h"
#include "es8311_codec.h"
#include "esp_codec_dev.h"

void xiaozhi_audio_i2c_init(void);

void xiaozhi_audio_i2s_init(void);

void xiaozhi_audio_init(void);

void xiaozhi_audio_play(void *buf, uint32_t size);

void xiaozhi_audio_record(void *buf, uint32_t size);

void xiaozhi_audio_open(void);

void xiaozhi_audio_close(void);

void xiaozhi_audio_set_volume(uint8_t volume);
/* 设置是否静音 */
void xiaozhi_audio_mute_set(bool mute);