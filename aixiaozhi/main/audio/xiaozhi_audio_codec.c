#include "xiaozhi_audio_codec.h"
//! 将PCM数据编码成Opus格式，并且取出
#define TAG "xiaozhi_audio_codec"
uint8_t *pcm_data = NULL;
uint8_t *opus_output_data = NULL;
esp_audio_enc_handle_t encoder = NULL; // 编码器句柄
int pcm_input_size = 0;                // 获取处理一帧PCM数据所需的字节数
int opus_output_size = 0;              // 获取编码后一帧数据所需的字节数

void xiaozhi_audio_codec_task(void *pvParameters);
void xiaozhi_audio_codec_init(void)
{
    esp_opus_enc_register();
    //  Configuration for opus encoder
    esp_opus_enc_config_t opus_cfg = {
        .sample_rate = ESP_AUDIO_SAMPLE_RATE_16K,
        .channel = ESP_AUDIO_MONO,
        .bits_per_sample = ESP_AUDIO_BIT16,
        .bitrate = 32000,
        .frame_duration = ESP_OPUS_ENC_FRAME_DURATION_60_MS,
        .application_mode = ESP_OPUS_ENC_APPLICATION_AUDIO,
        .complexity = 5,
        .enable_fec = false,
        .enable_dtx = false,
        .enable_vbr = false,
    };
    // esp_audio_enc_config_t enc_cfg = {
    //     .type = ESP_AUDIO_TYPE_OPUS,
    //     .cfg = &opus_cfg,
    //     .cfg_sz = sizeof(opus_cfg)};

    /* 打开编码器 */
    esp_opus_enc_open(&opus_cfg, sizeof(esp_opus_enc_config_t), &encoder);

    // Get needed buffer size and prepare memory
    /* pcm_input_size :输出参数，返回处理一帧PCM数据所需的输入缓冲区大小
    &opus_output_size - 输出参数，返回编码后一帧数据所需的输出缓冲区大小

    这个函数会根据编码器的配置（如采样率、声道数、比特率等）来计算出处理一帧音频数据时：

    需要多少PCM数据作为输入（pcm_input_size）
    编码后最多会产生多少字节的OPUS数据（opus_output_size）*/
    esp_opus_enc_get_frame_size(encoder, &pcm_input_size, &opus_output_size); //! 混用api了会到这里卡死
    ESP_LOGW(TAG, "pcm_input_size:%d, opus_output_size:%d", pcm_input_size, opus_output_size);
    pcm_data = heap_caps_malloc(pcm_input_size, MALLOC_CAP_SPIRAM);           // 为PCM（未压缩的原始音频数据）分配内存
    opus_output_data = heap_caps_malloc(opus_output_size, MALLOC_CAP_SPIRAM); // 为编码后的数据分配内存
    xTaskCreatePinnedToCoreWithCaps(xiaozhi_audio_codec_task, "xiaozhi_audio_codec_task", 32 * 1024, NULL, 5, NULL, 1, MALLOC_CAP_SPIRAM);
}

/*编码器任务*/
void xiaozhi_audio_codec_task(void *pvParameters)
{
    esp_audio_enc_in_frame_t in_frame = {
        .buffer = pcm_data,    // 指向存放未压缩PCM音频数据的缓冲区的起始地址
        .len = pcm_input_size, // 表示缓冲区中数据的长度，差不多1920字节
    };
    // 经过编码器处理后的数据存放在out_frame中，这个数据是经过加工后的数据
    esp_audio_enc_out_frame_t out_frame = {
        .buffer = opus_output_data,
        .len = opus_output_size,
    };
    while (1)
    {
        size_t remain_size = in_frame.len;         // 当前剩余的pcm数据大小
        uint8_t *cur_input_data = in_frame.buffer; // 从这个地址开始取出数据
        /*当剩余的pcm数据大于0时，才把数据环形缓冲区中提取出来*/
        while (remain_size > 0)
        {
            size_t cur_recv_size = 0; // 环形缓冲区中实际提取到的数据大小
            /*从环形缓冲区中提取(pcm)数据，返回一个指针*/
            void *recv_pointer = xRingbufferReceiveUpTo(xiaozhi_handle.sr2coder_ringbuf_handle, &cur_recv_size, portMAX_DELAY, remain_size);
            /*将从缓冲区提取到的数据复制到cur_input_data中*/
            memcpy(cur_input_data, recv_pointer, cur_recv_size);
            remain_size -= cur_recv_size;

            cur_input_data += cur_recv_size; // 下一次从这里放数据

            /*释放recv_pointer指向的数据*/
            vRingbufferReturnItem(xiaozhi_handle.sr2coder_ringbuf_handle, recv_pointer);
        }
        /*取完pcm，或者超时，开始编码*/
        esp_opus_enc_process(encoder, &in_frame, &out_frame); //! 这里api又错了
        // 编码也是需要时间的，所以我们用out_frame.encoded_bytes知道编码了多少数据
        //  todo 将编码后的数据发给另一个环形缓冲区
        //  out_frame.encoded_bytes已经编码过的数据
        xRingbufferSend(xiaozhi_handle.coder2ws_ringbuf_handle, out_frame.buffer, out_frame.encoded_bytes, portMAX_DELAY);
    }
}

