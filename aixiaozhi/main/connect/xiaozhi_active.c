#include "xiaozhi_active.h"
//!https请求
esp_err_t _http_event_handler(esp_http_client_event_t *evt);
void xiaozhi_active_setHeader(void);
void xiaozhi_active_setBody(void);
void generate_uuid_v4(char *uuid_str);
void xiaozhi_active_parse_response(char *response);

#define TAG "xiaozhi_active"

// http客户端的句柄
esp_http_client_handle_t client;

uint8_t mac[6];
char mac_str[18];
char uuid_str[37];

static char *output_buffer = NULL; // Buffer to store response of http request from event handler
static int output_len = 0;         // Stores number of bytes read
static int current_index = 0;      // 标识拷贝的位置
char *post_data;

void xiaozhi_active_request(void)
{
    // http客户端配置
    esp_http_client_config_t config = {
        .host = "api.tenclass.net",
        .path = "/xiaozhi/ota/",
        .transport_type = HTTP_TRANSPORT_OVER_SSL, // https请求，会进行ssl加密
        .event_handler = _http_event_handler,
        // .cert_pem = howsmyssl_com_root_cert_pem_start,
        .crt_bundle_attach = esp_crt_bundle_attach, // https需要安全证书
        .method = HTTP_METHOD_POST,
        //   .timeout_ms = 15000,

    };
    client = esp_http_client_init(&config);

    // 设置请求头
    xiaozhi_active_setHeader();
    // 设置请求体
    xiaozhi_active_setBody();
    // 发请求，最多重试3次
    int retry_count = 0;
    esp_err_t err;
    xiaozhi_lvgl_title_label_update("请求网络中...");
    xiaozhi_lvgl_emoji_label_update("embarrassed");
    xiaozhi_lvgl_center_text_update("请稍等...");

    while ((err = esp_http_client_perform(client)) != ESP_OK && retry_count < 3)
    {
        ESP_LOGE(TAG, "HTTP request failed: %s, retry %d/3", esp_err_to_name(err), retry_count + 1);
        retry_count++;
        vTaskDelay(pdMS_TO_TICKS(2000)); // 等待2秒后重试
    }

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP request failed after 3 retries");
        xiaozhi_lvgl_title_label_update("网络请求失败");
        xiaozhi_lvgl_emoji_label_update("sad");
        xiaozhi_lvgl_center_text_update("请检查网络连接");
    }

    // esp_err_t err = esp_http_client_perform(client);
}

void xiaozhi_active_setHeader(void)
{
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Host", "api.tenclass.net");
    esp_http_client_set_header(client, "User-Agent", "bread-compact-wifi-128x64/1.0.1");
    // 获取mac地址
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    esp_http_client_set_header(client, "Device-Id", mac_str);
    ESP_LOGI(TAG, "mac_str: %s", mac_str);

    // 设置client_id: uuid
    generate_uuid_v4(uuid_str);
    esp_http_client_set_header(client, "Client-Id", uuid_str);
    ESP_LOGI(TAG, "uuid_str: %s", uuid_str);
}
void xiaozhi_active_setBody(void)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *application = cJSON_CreateObject();
    cJSON *board = cJSON_CreateObject();

    // 将application和board添加至root
    cJSON_AddItemToObject(root, "application", application);
    cJSON_AddItemToObject(root, "board", board);

    /* 给application添加成员属性 */
    cJSON_AddStringToObject(application, "version", "1.0.1");
    cJSON_AddStringToObject(application, "elf_sha256", esp_app_get_elf_sha256_str());

    // 给board添加成员属性
    /*
      根据一下数据给board添加成员属性
          "type": "bread-compact-wifi",
      "name": "bread-compact-wifi-128x64",
      "ssid": "卧室",
      "rssi": -55,
      "channel": 1,
      "ip": "192.168.1.11",
      "mac": "11:22:33:44:55:66"
    */
    cJSON_AddStringToObject(board, "type", "bread-compact-wifi");
    cJSON_AddStringToObject(board, "name", "bread-compact-wifi-128x64");
    cJSON_AddStringToObject(board, "ssid", "卧室");
    cJSON_AddNumberToObject(board, "rssi", -55);
    cJSON_AddNumberToObject(board, "channel", 1);
    cJSON_AddStringToObject(board, "ip", "192.168.1.11");
    cJSON_AddStringToObject(board, "mac", mac_str);

    // 转换成字符串
    post_data = cJSON_PrintUnformatted(root);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    // 删除cjson结构体
    cJSON_Delete(root);
    // ! 不应该在此释放post_data, 应该在接收到数据以后，
}
void generate_uuid_v4(char *uuid_str)
{
    // UUID v4 格式: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
    // 其中：x是随机十六进制数，y是8,9,A,B中的一个

    uint8_t uuid[16];

    // 生成16字节随机数
    for (int i = 0; i < 16; i++)
    {
        uuid[i] = esp_random() & 0xFF;
    }

    // 设置版本位为4 (UUID v4)
    uuid[6] = (uuid[6] & 0x0F) | 0x40;

    // 设置变体位为10xx (RFC 4122)
    uuid[8] = (uuid[8] & 0x3F) | 0x80;

    // 格式化为字符串
    snprintf(uuid_str, 37,
             "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             uuid[0], uuid[1], uuid[2], uuid[3],
             uuid[4], uuid[5], uuid[6], uuid[7],
             uuid[8], uuid[9], uuid[10], uuid[11],
             uuid[12], uuid[13], uuid[14], uuid[15]);
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{

    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    // ! 接收到响应头
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        // 判断响应头是否是Content-Length
        if (strcmp(evt->header_key, "Content-Length") == 0)
        {
            // 动态为output_buffer分配内存
            // malloc只负责分配内存，但是分配内存中可能有脏数据， calloc会将分配的内存空间都清空
            output_len = atoi(evt->header_value); // 将字符串转成number
            output_buffer = heap_caps_calloc(output_len + 1, sizeof(char), MALLOC_CAP_SPIRAM);
        }
        break;
    // ! 接收到响应数据
    // 分多次接收数据
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        memcpy(output_buffer + current_index, evt->data, evt->data_len);
        current_index += evt->data_len;
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        // ! 打印最终接收到的数据
        ESP_LOGI(TAG, "output_len, %d, output_buffer: %s", output_len, output_buffer);
        // 解析返回的json数据
        xiaozhi_active_parse_response(output_buffer);
        cJSON_free(post_data);

        if (output_buffer != NULL)
        {
            // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
            // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
            free(output_buffer);
            output_buffer = NULL;
            output_len = 0;
            current_index = 0;
        }
        output_len = 0;
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        int mbedtls_err = 0;
        esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
        if (err != 0)
        {
            ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
            ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
        }
        if (output_buffer != NULL)
        {
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
        esp_http_client_set_header(evt->client, "From", "user@example.com");
        esp_http_client_set_header(evt->client, "Accept", "text/html");
        esp_http_client_set_redirection(evt->client);
        break;
    }
    return ESP_OK;
}
/**
 * @brief 提取返回的json数据
 *
 * @param response 返回的json字符串
 */
void xiaozhi_active_parse_response(char *response)
{
    // 将json字符串转换成json对象，之后就可以使用cJSON_GetObjectItem函数提取数据
    cJSON *res = cJSON_Parse(response);
    // 提取websocket对象
    cJSON *websocket = cJSON_GetObjectItem(res, "websocket");
    // 判断是否有websocket对象
    if (websocket == NULL)
    {
        ESP_LOGW(TAG, "no websocket object");
        return;
    }

    char *websocket_url = cJSON_GetObjectItem(websocket, "url")->valuestring; // 获取websocket的url的字符串
    xiaozhi_handle.websocket_url = heap_caps_calloc(strlen(websocket_url) + 1, sizeof(char), MALLOC_CAP_SPIRAM);
    memcpy(xiaozhi_handle.websocket_url, websocket_url, strlen(websocket_url));
    // 不要使用浅拷贝， 会导致两个指针指向同一块内存
    // xiaozhi_handle.websocket_url = websocket_url;

    char *websocket_token = cJSON_GetObjectItem(websocket, "token")->valuestring;
    xiaozhi_handle.websocket_token = heap_caps_calloc(strlen(websocket_token) + 1, sizeof(char), MALLOC_CAP_SPIRAM);
    memcpy(xiaozhi_handle.websocket_token, websocket_token, strlen(websocket_token));

    // !判断当前的设备是否已经激活
    // 已经激活的话就没有这个activation对象了
    cJSON *activation = cJSON_GetObjectItem(res, "activation");
    if (activation == NULL)
    {
        xiaozhi_handle.is_actived = true;
        ESP_LOGW(TAG, "设备已经激活");
        xiaozhi_lvgl_title_label_update(AI_NAME);
        xiaozhi_lvgl_emoji_label_update("loving");
        xiaozhi_lvgl_center_text_update("李崇昊,你来了!");
    }
    else
    {
        xiaozhi_handle.is_actived = false;

        ESP_LOGW(TAG, "设备未激活");
        char *active_code = cJSON_GetObjectItem(activation, "code")->valuestring;
        xiaozhi_handle.active_code = heap_caps_calloc(strlen(active_code) + 1, sizeof(char), MALLOC_CAP_SPIRAM);
        memcpy(xiaozhi_handle.active_code, active_code, strlen(active_code));

        char text[100];
        xiaozhi_lvgl_title_label_update("设备未激活");
        xiaozhi_lvgl_emoji_label_update("sad");
        sprintf(text, "激活码: %s", xiaozhi_handle.active_code);
        xiaozhi_lvgl_center_text_update(text);
    }

    ESP_LOGI(TAG, "websocket_url: %s", xiaozhi_handle.websocket_url);
    ESP_LOGI(TAG, "websocket_token: %s", xiaozhi_handle.websocket_token);
    ESP_LOGI(TAG, "is_actived: %d", xiaozhi_handle.is_actived);
    if (!xiaozhi_handle.is_actived)
    {
        ESP_LOGI(TAG, "active_code: %s", xiaozhi_handle.active_code);
    }
    cJSON_Delete(res);
}