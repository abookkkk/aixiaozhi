#include "xiaozhi_audio_decode.h"
//! 从环形缓冲区取数据，进行音频解码

esp_audio_dec_handle_t dec_handle = NULL;
void xiaozhi_audio_decode_task(void *arg);
void xiaozhi_audio_decode_init(void)
{
    // 1. 初始化音频解码器
    esp_opus_dec_cfg_t opus_cfg = {
        .sample_rate = ESP_AUDIO_SAMPLE_RATE_16K,
        .channel = ESP_AUDIO_MONO,
        .frame_duration = ESP_OPUS_DEC_FRAME_DURATION_60_MS,
        .self_delimited = false,
    };
    // 2. 开启音频解码器
    esp_opus_dec_open(&opus_cfg, sizeof(opus_cfg), &dec_handle);

    xTaskCreatePinnedToCoreWithCaps(xiaozhi_audio_decode_task, "xiaozhi_audio_decode_task", 20 * 1024, NULL, 16, NULL, 0, MALLOC_CAP_SPIRAM);
}

/*解码器的任务*/
void xiaozhi_audio_decode_task(void *arg)
{
    // 要输入解码器的数据
    esp_audio_dec_in_raw_t in_raw = {
        .buffer = NULL,
        .len = 0,
        .consumed = 0,
    };
    // 加工完成后的数据
    esp_audio_dec_out_frame_t out_frame = {
        .buffer = heap_caps_malloc(8 * 1024, MALLOC_CAP_SPIRAM), // 16000 * 1 * 60 * 2 * 2 / 1000
        .len = 8 * 1024,                                         // Single packet size  16000 * 1 * 60 * 2 / 1000
    };
    while (1)
    {

        void *recv_pointer = xRingbufferReceive(xiaozhi_handle.ws2decoder_ringbuf_handle, &in_raw.len, portMAX_DELAY); // 这里当时没写&in_raw.len
        in_raw.buffer = recv_pointer;
        esp_audio_dec_info_t dec_info = {0};

        while (in_raw.len > 0)
        {
            // 解码
            esp_opus_dec_decode(dec_handle, &in_raw, &out_frame, &dec_info);
            in_raw.len -= in_raw.consumed;    // 更新输入数据
            in_raw.buffer += in_raw.consumed; // 更新指针

            // 解码后的数据使用音频播放
            xiaozhi_audio_play(out_frame.buffer, out_frame.decoded_size);//这个长度当时写错了
        }
        // 释放内存
        vRingbufferReturnItem(xiaozhi_handle.ws2decoder_ringbuf_handle, recv_pointer);
    }
}