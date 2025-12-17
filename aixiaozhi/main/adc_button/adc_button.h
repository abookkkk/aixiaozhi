#ifndef __ADC_BUTTON_H__
#define __ADC_BUTTON_H__
#include "esp_log.h"
#include "iot_button.h"
#include "button_adc.h"

#define TAG "adc_button"

void adc_button_init(void);
/**
 * @brief 注册按键2的回调函数
 *
 * @param event 触发事件
 * @param cb 要执行的回调
 * @param user_data 要传给回调的数据
 */
void adc_button_register_sw2_cb(button_event_t event, button_cb_t cb, void *user_data);
/**
 * @brief 注册按键3的回调函数
 *
 * @param event 触发事件
 * @param cb 要执行的回调
 * @param user_data 要传给回调的数据
 */
void adc_button_register_sw3_cb(button_event_t event, button_cb_t cb, void *user_data);
#endif /* __ADC_BUTTON_H__ */
