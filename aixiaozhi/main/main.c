#include <stdio.h>
#include "adc_button.h"
#include "connect\xiaozhi_wifi.h"
#include "connect\xiaozhi_active.h"
#include "connect\xiaozhi_ws.h"
#include "LCD\xiaozhi_lcd.h"
#include "LCD\xiaozhi_lvgl.h"
#include "audio\xiaozhi_audio.h"
#include "audio\xiaozhi_sr.h"
#include "audio\xiaozhi_audio_codec.h"
#include "audio\xiaozhi_audio_decode.h"
// extern esp_websocket_client_handle_t ws_client;
#define TAG "main_button"

void adc_button_cb(void *button_handle, void *usr_data);
void xiaozhi_wakeup_callback(void);                        // 唤醒的回调函数
void xiaozhi_vad_changed_cb(void);                         // 人声状态改变的回调函数
void xiaozhi_ws_text_process_cb(char *text, int text_len); // websocket的文本处理回调函数
void xiaozhi_ws_binary_process_cb(uint8_t *data, int data_len);

static void create_ringbuf_for_xiaozhi(void);
void encoder2decoder_task(void *pvParameters);
void ringbuf2ws_task(void *pvParameters);

/* 定义小智的结构体对象 */
XiaoZhi_Handle_t xiaozhi_handle;
typedef enum
{
    SW2_SINGLE_CLICK,
    SW3_SINGLE_CLICK,
    SW3_LONG_PRESS_START
} button_cb;

void app_main(void)
{

    adc_button_init();
    //  lcd_app_main();
    // 创建小智的事件组
    xiaozhi_handle.xiaozhi_event_group = xEventGroupCreate();
    // 注册唤醒的回调函数
    xiaozhi_handle.xiaozhi_wakeup_callback = xiaozhi_wakeup_callback;
    // 注册人声状态改变的回调函数
    xiaozhi_handle.xiaozhi_vad_changed_callback = xiaozhi_vad_changed_cb;
    // 注册websocket的文本处理回调函数，就释放了一个hello的标志位
    xiaozhi_handle.ws_text_proccess = xiaozhi_ws_text_process_cb;
    // 注册websocket的二进制处理回调函数
    xiaozhi_handle.ws_binary_proccess = xiaozhi_ws_binary_process_cb;
    // 按键的回调
    adc_button_register_sw2_cb(BUTTON_SINGLE_CLICK, adc_button_cb, (void *)SW2_SINGLE_CLICK);
    adc_button_register_sw3_cb(BUTTON_SINGLE_CLICK, adc_button_cb, (void *)SW3_SINGLE_CLICK);
    adc_button_register_sw3_cb(BUTTON_LONG_PRESS_START, adc_button_cb, (void *)SW3_LONG_PRESS_START);
    xiaozhi_lvgl_init();

    xiaozhi_wifi_init();

    // xiaozhi_lcd_init();
    // xiaozhi_lcd_test();

    // https
    xiaozhi_active_request();

    create_ringbuf_for_xiaozhi();
    /*设置小智的状态为空闲*/
    xiaozhi_handle.xiaozhi_state = XIAOZHI_STATE_IDLE;
    xiaozhi_sr_init();
    // ESP_LOGI(TAG, "Free heap size: %ld", esp_get_free_heap_size());
    xiaozhi_ws_init();

    //! 测试环形编码解码
    // xiaozhi_lvgl_helloworld();

    xiaozhi_audio_codec_init();
    xiaozhi_audio_decode_init();

    // xTaskCreatePinnedToCoreWithCaps(encoder2decoder_task, "encoder2decoder_task", 16 * 1024, NULL, 5, NULL, 0, MALLOC_CAP_SPIRAM);
    xTaskCreatePinnedToCoreWithCaps(ringbuf2ws_task, "ringbuf2ws_task", 16 * 1024, NULL, 5, NULL, 0, MALLOC_CAP_SPIRAM);
    //! 测试录音
    // xiaozhi_audio_init();
    // uint8_t data[1024];
    // while (1)
    // {
    //     xiaozhi_audio_record(data, 1024);
    //     xiaozhi_audio_play(data, 1024);
    // }
}

void adc_button_cb(void *button_handle, void *usr_data)
{
    uint8_t event_type = (uint8_t)usr_data;
    switch (event_type)
    {
    case SW2_SINGLE_CLICK:
        ESP_LOGI(TAG, "SW2_SINGLE_CLICK");
        break;
    case SW3_SINGLE_CLICK:
        ESP_LOGI(TAG, "SW3_SINGLE_CLICK");
        break;
    case SW3_LONG_PRESS_START:
        ESP_LOGI(TAG, "SW3_LONG_PRESS_START");
        // 重置wifi 配网
        xiaozhi_wifi_restart();
        break;

    default:
        break;
    }
}

static void create_ringbuf_for_xiaozhi()
{
    /*创建从录音到编码器的环形缓冲区*/
    xiaozhi_handle.sr2coder_ringbuf_handle = xRingbufferCreateWithCaps(32 * 1024, RINGBUF_TYPE_BYTEBUF, MALLOC_CAP_SPIRAM);
    /*创建从编码器到websocket的环形缓冲区*/
    xiaozhi_handle.coder2ws_ringbuf_handle = xRingbufferCreateWithCaps(32 * 1024, RINGBUF_TYPE_NOSPLIT, MALLOC_CAP_SPIRAM);
    /*创建从websocket到解码器的环形缓冲区*/
    xiaozhi_handle.ws2decoder_ringbuf_handle = xRingbufferCreateWithCaps(32 * 1024, RINGBUF_TYPE_NOSPLIT, MALLOC_CAP_SPIRAM);
}
void encoder2decoder_task(void *pvParameters)
{
    while (1)
    {
        // 从编码器到ws的环形缓冲区取数据
        size_t size = 0;
        uint8_t *data_buff = (uint8_t *)xRingbufferReceive(xiaozhi_handle.coder2ws_ringbuf_handle, &size, portMAX_DELAY);
        // (测试)从ws到解码器的环形缓冲区写入数据
        // xRingbufferSend(xiaozhi_handle.ws2decoder_ringbuf_handle, data_buff, size, portMAX_DELAY);
        vRingbufferReturnItem(xiaozhi_handle.coder2ws_ringbuf_handle, data_buff);
    }
}

void xiaozhi_wakeup_callback(void)
{
    ESP_LOGI(TAG, "被唤醒了");

    if (xiaozhi_handle.xiaozhi_state == XIAOZHI_STATE_IDLE)
    {
        bool connect_result = xiaozhi_ws_create_channle();
        if (connect_result == true)
        {
            // 要先发送hello，才能接下来发送后面的
            ws_send_hello();
            // ! 再次发送ioe的控制命令更有意义
            xiaozhi_ws_send_iot_command();
            xiaozhi_ws_send_iot_state();
            // 等待hello响应
            EventBits_t hello_res = xEventGroupWaitBits(xiaozhi_handle.xiaozhi_event_group, WS_HELLO_RESPONSE_BIT, pdTRUE, pdFALSE, portMAX_DELAY);

            if (hello_res & WS_HELLO_RESPONSE_BIT)
            {
                // 可以发送唤醒词了
                xiaozhi_ws_send_wake_word();
            }
            else
            {
                ESP_LOGI(TAG, "hello响应超时");
            }
        }
    }
    /*如果小智已经被唤醒了，我又发送唤醒词，并且小智在说话，那么小智会终止说话，状态改为idle*/
    else if (xiaozhi_handle.xiaozhi_state == XIAOZHI_STATE_SPEAKING)
    {
        // 清空coder2ws缓冲区

        xiaozhi_ws_send_stop();
        // xiaozhi_handle.xiaozhi_state = XIAOZHI_STATE_IDLE;
        while (1)
        {
            size_t size = 0;
            // 目的是消耗coder2ws_ringbuf环形缓冲区里面的数据,减少那种拖长的音节
            void *data = xRingbufferReceive(xiaozhi_handle.coder2ws_ringbuf_handle, &size, pdMS_TO_TICKS(10));
            if (data == NULL)
            {
                break;
            }
            vRingbufferReturnItem(xiaozhi_handle.coder2ws_ringbuf_handle, data);
        }
    }
}
/*接收到文本*/
void xiaozhi_ws_text_process_cb(char *text, int text_len)
{
    // 根路径
    cJSON *root = cJSON_Parse(text);
    if (root == NULL)
    {
        ESP_LOGW(TAG, "解析json失败");
        return;
    }
    cJSON *type = cJSON_GetObjectItem(root, "type");
    if (type == NULL)
    {
        ESP_LOGW(TAG, "没有type字段");
        return;
    }
    if (strcmp(type->valuestring, "hello") == 0)
    {
        // 修改事件标志组中hello的标志位
        xEventGroupSetBits(xiaozhi_handle.xiaozhi_event_group, WS_HELLO_RESPONSE_BIT);
    }
    //判断是否是语音转文本
    if (strcmp(type->valuestring, "stt") == 0)
    { 
        cJSON *text = cJSON_GetObjectItem(root, "text");
        xiaozhi_lvgl_title_label_update(text->valuestring);
    }
    // 判断是否是文本帧
    if (strcmp(type->valuestring, "tts") == 0)
    {
        cJSON *state = cJSON_GetObjectItem(root, "state");
        if (strcmp(state->valuestring, "sentence_start") == 0)
        {
            cJSON *text = cJSON_GetObjectItem(root, "text");
            xiaozhi_lvgl_center_text_update(text->valuestring);
        }

        if (strcmp(state->valuestring, "start") == 0)
        {
            // 开始说话
            xiaozhi_handle.xiaozhi_state = XIAOZHI_STATE_SPEAKING;
            xiaozhi_lvgl_title_label_update("正在说话");
        }
        if (strcmp(state->valuestring, "stop") == 0)
        {
            // 停止说话
            xiaozhi_handle.xiaozhi_state = XIAOZHI_STATE_IDLE;
            xiaozhi_lvgl_title_label_update(AI_NAME);
        }
    }
    /*是否是iot*/
    if (strcmp(type->valuestring, "iot") == 0)
    {
        cJSON *commands = cJSON_GetObjectItem(root, "commands");
        cJSON *cmdItem = cJSON_GetArrayItem(commands, 0);
        cJSON *parametes = cJSON_GetObjectItem(cmdItem, "parameters");

        char *method = cJSON_GetObjectItem(cmdItem, "method")->valuestring;

        if (strcmp(method,"SetVolume")==0)
        {
            int volume = cJSON_GetObjectItem(parametes, "volume")->valueint;
            xiaozhi_audio_set_volume(volume);
        }
        else if (strcmp(method,"SetMute")==0)
        {
            cJSON *muteItem = cJSON_GetObjectItem(parametes, "mute");
            bool is_mute = false;
            if (cJSON_IsBool(muteItem))
            {
                is_mute = cJSON_IsTrue(muteItem);
            }
            xiaozhi_audio_mute_set(is_mute);
        }
    }
    // 是否是表情
    if (strcmp(type->valuestring, "llm") == 0)
    {
        cJSON *emotion = cJSON_GetObjectItem(root, "emotion");
        xiaozhi_lvgl_emoji_label_update(emotion->valuestring);
    }
    // 删除根节点
    cJSON_Delete(root);
}

void xiaozhi_ws_binary_process_cb(uint8_t *data, int data_len)
{
    // 将这些数据放入解码器的环形缓冲区中
    xRingbufferSend(xiaozhi_handle.ws2decoder_ringbuf_handle, data, data_len, portMAX_DELAY);
}

void ringbuf2ws_task(void *pvParameters)
{
    while (1)
    {
        // 从编码器到ws的环形缓冲区取数据
        size_t size = 0;
        char *data_buff = (char *)xRingbufferReceive(xiaozhi_handle.coder2ws_ringbuf_handle, &size, portMAX_DELAY);
        /*xiaozhi的状态为listening时再发送数据到websocket*/
        if (xiaozhi_handle.xiaozhi_state == XIAOZHI_STATE_LISTENING)
        {
            xiaozhi_ws_SendBinary(data_buff, size);
        }

        vRingbufferReturnItem(xiaozhi_handle.coder2ws_ringbuf_handle, data_buff);
    }
}

void xiaozhi_vad_changed_cb(void)
{

    ESP_LOGI("vad_test", "vad status : %s", xiaozhi_handle.current_vad_state == VAD_SPEECH ? "说话中" : "静音中");

    /*当前我正在说话，并且小智不在监听状态，则将小智的状态改为监听*/
    if (xiaozhi_handle.current_vad_state == VAD_SPEECH)
    {
        if (xiaozhi_handle.xiaozhi_state == XIAOZHI_STATE_IDLE)
        {
            xiaozhi_ws_send_listen();
            xiaozhi_handle.xiaozhi_state = XIAOZHI_STATE_LISTENING;
            xiaozhi_lvgl_title_label_update("宝儿正在聆听...");

        }
    }
    /*当前我没有说话，并且小智没在说话，将小智的状态改为空闲*/
    if (xiaozhi_handle.current_vad_state == VAD_SILENCE)
    {
        if (xiaozhi_handle.xiaozhi_state != XIAOZHI_STATE_SPEAKING)
        {
            xiaozhi_ws_send_stop_listen();
            xiaozhi_handle.xiaozhi_state = XIAOZHI_STATE_IDLE;
            xiaozhi_lvgl_title_label_update(AI_NAME);
        }
    }
}


