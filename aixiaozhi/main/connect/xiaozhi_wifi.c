#include "xiaozhi_wifi.h"

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

#define TAG "xiaozhi_wifi"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
// #define EXAMPLE_ESP_WIFI_SSID "Mate 70 Pro"
// #define EXAMPLE_ESP_WIFI_PASS "12345678"
#define PROV_QR_VERSION "v1"
#define PROV_TRANSPORT_BLE "ble"
#define QRCODE_BASE_URL "https://espressif.github.io/esp-jumpstart/qrcode.html"

// const int WIFI_CONNECTED_EVENT = BIT0;
// static EventGroupHandle_t wifi_event_group;
static int retries = 0;
void wifi_app_main(void);
void wifi_init_sta(void);
void xiaozhi_wifi_nvs_init(void);
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data);
static void get_device_service_name(char *service_name, size_t max);

static void wifi_prov_print_qr(const char *name, const char *username, const char *pop, const char *transport);
void wifi_init_sta(void)
{
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void xiaozhi_wifi_init(void)
{
    // 初始化 NVS
    xiaozhi_wifi_nvs_init();
    wifi_app_main();
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
void wifi_app_main(void)
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
    // 注册事件处理函数
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(PROTOCOMM_TRANSPORT_BLE_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(PROTOCOMM_SECURITY_SESSION_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    // 注册WIFI事件处理函数
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));
    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_ble,                                        // 选择蓝牙模式配网
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM}; // 配网结束自动释放蓝牙相关的所有信息
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

    // 检查设备是否已配置
    bool provisioned = false;

    /* Let's find out if the device is provisioned */
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));
    // 如果项目没有配置完成，则进行配置
    if (!provisioned)
    {
        ESP_LOGI(TAG, "Starting provisioning");

        /* What is the Device Service Name that we want
         * This translates to :
         *     - Wi-Fi SSID when scheme is wifi_prov_scheme_softap
         *     - device name when scheme is wifi_prov_scheme_ble
         */
        char service_name[12];
        get_device_service_name(service_name, sizeof(service_name));

        /*配置非对称密码*/
        wifi_prov_security_t security = WIFI_PROV_SECURITY_1;

        /* 密钥*/
        const char *pop = "abcd1234";

        wifi_prov_security1_params_t *sec_params = pop;

        const char *username = NULL;
        const char *service_key = NULL;

        /* 设备的自定义服务 UUID */
        uint8_t custom_service_uuid[] = {
            /* LSB <---------------------------------------
             * ---------------------------------------> MSB */
            0xb4,
            0xdf,
            0x5a,
            0x1c,
            0x3f,
            0x6b,
            0xf4,
            0xbf,
            0xea,
            0x4a,
            0x82,
            0x03,
            0x04,
            0x90,
            0x1a,
            0x02,
        };
        wifi_prov_scheme_ble_set_service_uuid(custom_service_uuid);
        /* 开始提供服务 */
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, (const void *)sec_params, service_name, service_key));
        /* 打印二维码 */ // TODO 调用生成二维码的函数
        wifi_prov_print_qr(service_name, username, pop, PROV_TRANSPORT_BLE);
    }
    else
    {
        // 配置过网了
        ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi STA");

        wifi_prov_mgr_deinit();
        /* Start Wi-Fi station */
        wifi_init_sta();
    }
    // 我们手动配置的
    // wifi_config_t wifi_config = {
    //     .sta = {
    //         .ssid = EXAMPLE_ESP_WIFI_SSID,
    //         .password = EXAMPLE_ESP_WIFI_PASS,

    //         .threshold.authmode = WIFI_AUTH_WPA2_PSK, // ESP32 在扫描 Wi-Fi 网络时，
    //                                                   // 只会考虑那些认证模式不低于此阈值的网络;WPA2_PSK :8-63 字符

    //         .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
    //         .sae_h2e_identifier = "",
    //     },
    // };
    // ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    // ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT,
                                           pdTRUE,
                                           pdFALSE,
                                           portMAX_DELAY);
    // xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, true, true, portMAX_DELAY);
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
    if (event_base == WIFI_PROV_EVENT)
    {
        switch (event_id)
        {
        case WIFI_PROV_START:
            ESP_LOGI(TAG, "Provisioning started");
            break;
        case WIFI_PROV_CRED_RECV:
        {
            wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
            ESP_LOGI(TAG, "Received Wi-Fi credentials"
                          "\n\tSSID     : %s\n\tPassword : %s",
                     (const char *)wifi_sta_cfg->ssid,
                     (const char *)wifi_sta_cfg->password);
            break;
        }
        case WIFI_PROV_CRED_FAIL:
        {
            wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
            ESP_LOGE(TAG, "Provisioning failed!\n\tReason : %s"
                          "\n\tPlease reset to factory and retry provisioning",
                     (*reason == WIFI_PROV_STA_AUTH_ERROR) ? "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");

            retries++;
            if (retries >= 5)
            {
                ESP_LOGI(TAG, "Failed to connect with provisioned AP, resetting provisioned credentials");
                wifi_prov_mgr_reset_sm_state_on_failure();
                retries = 0;
            }

            break;
        }
        case WIFI_PROV_CRED_SUCCESS:
            ESP_LOGI(TAG, "Provisioning successful");
            // ! 配网成功 ，主动删除二维码
            xiaozhi_lvgl_del_qrcode();
            retries = 0;

            break;
        case WIFI_PROV_END:
            /* De-initialize manager once provisioning is finished */
            wifi_prov_mgr_deinit();
            break;
        default:
            break;
        }
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

    else if (event_base == PROTOCOMM_TRANSPORT_BLE_EVENT)
    {
        switch (event_id)
        {
        case PROTOCOMM_TRANSPORT_BLE_CONNECTED:
            ESP_LOGI(TAG, "BLE transport: Connected!");
            break;
        case PROTOCOMM_TRANSPORT_BLE_DISCONNECTED:
            ESP_LOGI(TAG, "BLE transport: Disconnected!");
            break;
        default:
            break;
        }
    }

    else if (event_base == PROTOCOMM_SECURITY_SESSION_EVENT)
    {
        switch (event_id)
        {
        case PROTOCOMM_SECURITY_SESSION_SETUP_OK:
            ESP_LOGI(TAG, "Secured session established!");
            break;
        case PROTOCOMM_SECURITY_SESSION_INVALID_SECURITY_PARAMS:
            ESP_LOGE(TAG, "Received invalid security parameters for establishing secure session!");
            break;
        case PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH:
            ESP_LOGE(TAG, "Received incorrect username and/or PoP for establishing secure session!");
            break;
        default:
            break;
        }
    }
}

static void get_device_service_name(char *service_name, size_t max)
{
    uint8_t eth_mac[6];
    const char *ssid_prefix = "PROV_";
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(service_name, max, "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}

static void wifi_prov_print_qr(const char *name, const char *username, const char *pop, const char *transport)
{
    char payload[150] = {0}; // 存放二维码数据
    if (!name || !transport)
    {
        ESP_LOGW(TAG, "Cannot generate QR code payload. Data missing.");
        return;
    }

    if (pop)
    {
        snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\""
                                           ",\"pop\":\"%s\",\"transport\":\"%s\"}",
                 PROV_QR_VERSION, name, pop, transport);
    }
    else
    {
        snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\""
                                           ",\"transport\":\"%s\"}",
                 PROV_QR_VERSION, name, transport);
    }

    ESP_LOGI(TAG, "Scan this QR code from the provisioning application for Provisioning.");

    xiaozhi_lvgl_title_label_update("请使用小智App扫码配网");
    xiaozhi_lvgl_qrcode_show(payload);
    esp_qrcode_config_t cfg = ESP_QRCODE_CONFIG_DEFAULT();
    // TODO 先不让他打印到控制台
    //esp_qrcode_generate(&cfg, payload);
    ESP_LOGI(TAG, "If QR code is not visible, copy paste the below URL in a browser.\n%s?data=%s", QRCODE_BASE_URL, payload);
}

void xiaozhi_wifi_restart(void)
{
    // 清除wifi配置信息
    esp_wifi_restore();
    // nvs_flash_erase()

    // 重启芯片
    esp_restart();
}