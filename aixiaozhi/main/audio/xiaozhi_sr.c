#include "xiaozhi_sr.h"
//! 将es8311的音频数据转换成pcm数据，喂给语音识别并提取数据
#define TAG "xiaozhi_sr"

static esp_afe_sr_iface_t *afe_handle = NULL; // AFE接口句柄
static esp_afe_sr_data_t *afe_data = NULL;    // 音频数据结构，包含处理所需的状态信息

void feed_Task(void *arg);
void detect_Task(void *arg);

void xiaozhi_sr_init(void)
{
    xiaozhi_audio_init();


    //xiaozhi_handle.xiaozhi_wakeup_callback = xiaozhi_wakeup_cb;
    // 选择我们需要的语音模型，加载到flash分区表中的"model"分区
    srmodel_list_t *models = esp_srmodel_init("model");
    // 可能会因为高性能触发看门狗，可以选择低功耗模式
    afe_config_t *afe_config = afe_config_init("M", models, AFE_TYPE_SR, AFE_MODE_HIGH_PERF);

    afe_config->vad_min_noise_ms = 1000; // 用于判断是否是噪音或者静默时间
    afe_config->vad_min_speech_ms = 128; // 判断为语音的最短时间
    afe_config->vad_mode = VAD_MODE_1;   // 识别的要求低一些，更好识别

    afe_config->aec_init = false; // 关闭回声消除
    afe_config->se_init = false;  // 关闭人声增强
    afe_config->ns_init = false;  // 关闭噪声消除

    afe_config->wakenet_mode = DET_MODE_95; // 设置唤醒模型的模式

    afe_config->memory_alloc_mode = AFE_MEMORY_ALLOC_MORE_PSRAM; // 使用更多的PSRAM

    afe_config->afe_ringbuf_size = 6 * 1024;

    afe_handle = esp_afe_handle_from_config(afe_config);
    afe_data = afe_handle->create_from_config(afe_config);
    // afe_config_free(afe_config);

    xTaskCreatePinnedToCoreWithCaps(feed_Task, "feed_Task", 8 * 1024, NULL, 5, NULL, 0, MALLOC_CAP_SPIRAM);
    xTaskCreatePinnedToCoreWithCaps(detect_Task, "detect_Task", 4 * 1024, NULL, 5, NULL, 1, MALLOC_CAP_SPIRAM);
}
void feed_Task(void *arg)
{
    /* 获取feed_chunksize,*/
    int feed_chunksize = afe_handle->get_feed_chunksize(afe_data);
    /* 获取通道数量M*/
    int nch = afe_handle->get_feed_channel_num(afe_data);

    int16_t *feed_buff = (int16_t *)malloc(feed_chunksize * sizeof(int16_t) * nch);
    // ESP_LOGW(TAG, "feed_buff size: %d", feed_chunksize * nch * sizeof(int16_t));

    while (1)
    {
        // esp_get_feed_data(true, feed_buff, feed_chunksize * sizeof(int16_t) * nch);
        /*从xiaozhi_audio里面获取语音数据*/
        xiaozhi_audio_record((void *)feed_buff, feed_chunksize * sizeof(int16_t) * nch);
        afe_handle->feed(afe_data, feed_buff);
    }
}
void detect_Task(void *arg)
{
    while (1)
    {
        /* 从音频前端引擎(AFE)获取处理后的结果 */
        afe_fetch_result_t *result = afe_handle->fetch(afe_data);
        /* 提取处理后的音频数据 */
        int16_t *processed_audio = result->data;
        //! 打印pcm音频数据的大小
        // ESP_LOGW(TAG, "processed_audio size: %d", result->data_size);
        /* 获取语音活动检测(VAD)状态，用于判断当前是否有语音活动 */
        vad_state_t vad_state = result->vad_state;
        /* 获取唤醒词检测状态，用于判断是否检测到唤醒词 */
        wakenet_state_t wakeup_state = result->wakeup_state;
        /*检测到唤醒词*/
        if (wakeup_state == WAKENET_DETECTED)
        {
            xiaozhi_handle.is_wakeup = true;

            if (xiaozhi_handle.xiaozhi_wakeup_callback != NULL)
            {
                xiaozhi_handle.xiaozhi_wakeup_callback();
            }
        }
        if (xiaozhi_handle.is_wakeup)
        {
            xiaozhi_handle.current_vad_state = vad_state;
            if (xiaozhi_handle.current_vad_state != xiaozhi_handle.last_vad_state)
            {
                // ESP_LOGI(TAG,"检测到语音状态改变");
                if (xiaozhi_handle.xiaozhi_vad_changed_callback != NULL)
                {
                    xiaozhi_handle.xiaozhi_vad_changed_callback();
                }
            }
            xiaozhi_handle.last_vad_state = xiaozhi_handle.current_vad_state;
        }
        /*!如果检测到唤醒状态和语音状态是说话，则将数据放入环形缓冲区*/
        if (xiaozhi_handle.is_wakeup && vad_state == VAD_SPEECH)
        {
            /*如果result->vad_cache_size > 0,这时候才开始放入数据*/
            if (result->vad_cache_size > 0)
            {
                xRingbufferSend(xiaozhi_handle.sr2coder_ringbuf_handle, result->vad_cache, result->vad_cache_size, portMAX_DELAY);
            }
            xRingbufferSend(xiaozhi_handle.sr2coder_ringbuf_handle, processed_audio, result->data_size, portMAX_DELAY);
        }
    }
}



