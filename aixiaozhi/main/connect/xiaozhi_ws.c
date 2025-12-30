#include "xiaozhi_ws.h"
char *iot_command_str =
#include "iot_data/iot_command.txt"
    ;
char *iot_state_str =
#include "iot_data/iot_state.txt"
    ;
extern char mac_str[18];
extern char uuid_str[37];
esp_websocket_client_handle_t ws_client;
void ws_send_hello(void);
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
bool xiaozhi_ws_create_channle(void);
#define TAG "xiaozhi_ws"

void xiaozhi_ws_init(void)
{
    esp_websocket_client_config_t websocket_cfg = {};
    /*设置请求的地址*/
    websocket_cfg.uri = xiaozhi_handle.websocket_url;
    /*设置ssl*/
    websocket_cfg.transport = WEBSOCKET_TRANSPORT_OVER_SSL;
    /*证书验证，不使用本地的证书*/
    websocket_cfg.crt_bundle_attach = esp_crt_bundle_attach;
    ESP_LOGI(TAG, "Connecting to %s...", xiaozhi_handle.websocket_url); //
    /*设置重连超时时间*/
    websocket_cfg.reconnect_timeout_ms = 2000;
    /*设置超时时间*/
    websocket_cfg.network_timeout_ms = 2000;

    /*初始化websocket客户端,返回一个客户端句柄*/
    ws_client = esp_websocket_client_init(&websocket_cfg);
    /*计算需要分配的内存大小*/
    size_t str_len = strlen("Bearer ") + strlen(xiaozhi_handle.websocket_token) + 1;

    char *auth_header = heap_caps_malloc(str_len, MALLOC_CAP_SPIRAM);
    sprintf(auth_header, "Bearer %s", xiaozhi_handle.websocket_token);
    /*这些是请求头*/
    esp_websocket_client_append_header(ws_client, "Authorization", auth_header);
    esp_websocket_client_append_header(ws_client, "Protocol-Version", "1");
    esp_websocket_client_append_header(ws_client, "Device-Id", mac_str);
    esp_websocket_client_append_header(ws_client, "Client-Id", uuid_str);
    /*注册事件处理函数，处理所有WebSocket事件，当事件发生时调用websocket_event_handler函数*/
    esp_websocket_register_events(ws_client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)ws_client);
}

/*websocket事件处理函数*/
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id)
    {
    case WEBSOCKET_EVENT_BEGIN:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_BEGIN");
        break;
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
        /*释放事件标志组的WS_CONNECT_SUCCESS_BIT位*/
        xEventGroupSetBits(xiaozhi_handle.xiaozhi_event_group, WS_CONNECT_SUCCESS_BIT);
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
        xiaozhi_handle.xiaozhi_state = XIAOZHI_STATE_IDLE;
        xiaozhi_handle.is_wakeup = false;
        break;
    case WEBSOCKET_EVENT_DATA:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
        ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
        // 文本数据
        if (data->op_code == WS_TRANSPORT_OPCODES_TEXT)
        {
            /*处理文本数据*/
            if (xiaozhi_handle.ws_text_proccess != NULL)
            {
                xiaozhi_handle.ws_text_proccess(data->data_ptr, data->data_len);
            }

            ESP_LOGI(TAG, "Got text data: %.*s", data->data_len, data->data_ptr);
        }
        // 二进制数据
        else if (data->op_code == WS_TRANSPORT_OPCODES_BINARY)
        {
            ESP_LOGI(TAG, "Got binary data len: %d", data->data_len);
            if (xiaozhi_handle.ws_binary_proccess != NULL)
            {
                xiaozhi_handle.ws_binary_proccess(data->data_ptr, data->data_len);
            }
        }
        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        xiaozhi_handle.xiaozhi_state = XIAOZHI_STATE_IDLE;
        xiaozhi_handle.is_wakeup = false;

        break;
    case WEBSOCKET_EVENT_FINISH:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_FINISH");
        xiaozhi_handle.xiaozhi_state = XIAOZHI_STATE_IDLE;
        xiaozhi_handle.is_wakeup = false;
        break;
    }
}
// 向websocket发送文本数据
void xiaozhi_ws_SendText(char *text)
{
    esp_websocket_client_send_text(ws_client, text, strlen(text), pdMS_TO_TICKS(5000));
}
// 向websocket发送二进制数据
void xiaozhi_ws_SendBinary(char *data, int data_len)
{
    esp_websocket_client_send_bin(ws_client, data, data_len, pdMS_TO_TICKS(5000));
}

bool xiaozhi_ws_create_channle(void)
{
    // 如果已经唤醒过了，直接返回true
    if (esp_websocket_client_is_connected(ws_client))
    {
        return true;
    }
    /*在被唤醒时启动WebSocket客户端，开始连接到服务器*/
    esp_websocket_client_start(ws_client);
    /*等到连接websocker成功后再发送hello包*/
    EventBits_t connect_result = xEventGroupWaitBits(xiaozhi_handle.xiaozhi_event_group, WS_CONNECT_SUCCESS_BIT, pdTRUE, pdTRUE, pdMS_TO_TICKS(5000));
    if (!(connect_result & WS_CONNECT_SUCCESS_BIT))
    {
        ESP_LOGI(TAG, "连接失败");
        return false;
    }

    return true;
}
/*发送hello包*/
void ws_send_hello(void)
{
    char *hello_text = "{\"type\":\"hello\",\"version\":1,\"transport\":\"websocket\",\"features\":{\"mcp\":true},\"audio_params\":{\"format\":\"opus\",\"sample_rate\":16000,\"channels\":1,\"frame_duration\":60}}";
    xiaozhi_ws_SendText(hello_text);
}
/*发送唤醒消息*/
void xiaozhi_ws_send_wake_word(void)
{
    char *wake_str = "{\"type\":\"listen\",\"state\":\"detect\",\"text\":\"你好, 宝儿\"}";
    xiaozhi_ws_SendText(wake_str);
}
/*发送开始监听消息*/
void xiaozhi_ws_send_listen(void)
{
    char *listen_str = "{\"type\":\"listen\",\"state\":\"start\",\"mode\":\"manual\"}";
    xiaozhi_ws_SendText(listen_str);
}
/*发送终止监听消息*/
void xiaozhi_ws_send_stop_listen(void)
{
    char *stop_listen_str = "{\"type\":\"listen\",\"state\":\"stop\"}";
    xiaozhi_ws_SendText(stop_listen_str);
}

/*发送中止消息*/
void xiaozhi_ws_send_stop(void)
{
    char *stop_str = "{\"type\":\"abort\",\"reason\":\"wake_word_detected\"}";
    xiaozhi_ws_SendText(stop_str);
}
/*发送iot的控制命令*/
void xiaozhi_ws_send_iot_command(void)
{
    xiaozhi_ws_SendText(iot_command_str);
}
/*发送iot的状态命令*/
void xiaozhi_ws_send_iot_state(void)
{
    xiaozhi_ws_SendText(iot_state_str);
}
