#include "adc_button.h"

//!gpio8
button_handle_t sw2_adc_btn = NULL;
button_handle_t sw3_adc_btn = NULL;

#define TAG "adc_button"

void adc_button_init(void)
{
    // create sw2adc button
    const button_config_t sw2_btn_cfg = {0};
    button_adc_config_t sw2_btn_adc_cfg = {
        .unit_id = ADC_UNIT_1,
        .adc_channel = 7,
        .button_index = 0,
        .min = 0,
        .max = 500, // 500mv
    };
    // create sw3adc button
    const button_config_t sw3_btn_cfg = {0};
    button_adc_config_t sw3_btn_adc_cfg = {
        .unit_id = ADC_UNIT_1,
        .adc_channel = 7,
        .button_index = 1,
        .min = 1300,
        .max = 1800, // 500mv
    };

    if (NULL == sw2_adc_btn)
    {
        ESP_LOGE(TAG, "Button2 create failed"); // ERROR
        // ESP_LOGW(TAG, "Button create failed"); // waring
        // ESP_LOGI(TAG, "Button create failed"); // info
        // ESP_LOGD(TAG, "Button create failed"); // debug
    }
    //
    iot_button_new_adc_device(&sw2_btn_cfg, &sw2_btn_adc_cfg, &sw2_adc_btn);

    if (NULL == sw3_adc_btn)
    {
        ESP_LOGE(TAG, "Button3 create failed"); // ERROR
        // ESP_LOGW(TAG, "Button3 create failed"); // waring
        // ESP_LOGI(TAG, "Button3 create failed"); // info
        // ESP_LOGD(TAG, "Button3 create failed"); // debug
    }
    //
    iot_button_new_adc_device(&sw3_btn_cfg, &sw3_btn_adc_cfg, &sw3_adc_btn);
}
/**
 * @brief 重写的注册按键2的回调函数
 *
 * @param event 触发事件
 * @param cb 要执行的回调
 * @param user_data 要传给回调的数据
 */
void adc_button_register_sw2_cb(button_event_t event, button_cb_t cb, void *user_data)
{
    iot_button_register_cb(sw2_adc_btn, event, NULL, cb, user_data);
}
/**
 * @brief 重写的注册按键3的回调函数
 *
 * @param event 触发事件
 * @param cb 要执行的回调
 * @param user_data 要传给回调的数据
 */
void adc_button_register_sw3_cb(button_event_t event, button_cb_t cb, void *user_data)
{
    iot_button_register_cb(sw3_adc_btn, event, NULL, cb, user_data);
}
