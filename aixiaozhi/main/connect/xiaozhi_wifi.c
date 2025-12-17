#include "xiaozhi_wifi.h"

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

#define TAG "xiaozhi_wifi"
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define EXAMPLE_ESP_WIFI_SSID "Mate 70 Pro"
#define EXAMPLE_ESP_WIFI_PASS "12345678"

void wifi_init_sta(void);
void xiaozhi_wifi_nvs_init(void);
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data);

void xiaozhi_wifi_init(void)
{
    // 初始化 NVS
    xiaozhi_wifi_nvs_init();
    wifi_init_sta();
}

/**
 * @brief 初始化WiFi NVS存储
 *
 * 该函数用于初始化NVS（Non-Volatile Storage）闪存，用于存储WiFi配置信息。
 * 如果NVS初始化失败且原因是无空闲页或版本不匹配，则擦除NVS并重新初始化。
 *
 * @param 无
 * @return 无
 */
void xiaozhi_wifi_nvs_init()
{
    // Initialize NVS 非遗失
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    { // 当NVS空间不足或发现新版本时，擦除NVS并重新初始化
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
}
void wifi_init_sta(void)
{
    // 创建事件标志组
    s_wifi_event_group = xEventGroupCreate();
    // 创建默认的Wi-Fi接口
    ESP_ERROR_CHECK(esp_netif_init());
    // 创建默认的Wi-Fi轮询事件的任务
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // 创建默认的Wi-Fi接口
    esp_netif_create_default_wifi_sta();
    // 创建Wi-Fi初始化配置
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    // 注册WIFI事件处理函数
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,

            .threshold.authmode = WIFI_AUTH_WPA2_PSK, // ESP32 在扫描 Wi-Fi 网络时，
                                                      //只会考虑那些认证模式不低于此阈值的网络;WPA2_PSK :8-63 字符

            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
            .sae_h2e_identifier = "",
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

/**
 * @brief wifi事件处理函数
 *
 * @param arg
 * @param event_base
 * @param event_id
 * @param event_data
 */
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < 5)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        // 一旦链接上AP， 获取到ip就放行
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}