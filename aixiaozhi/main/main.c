#include <stdio.h>
#include "adc_button.h"
#include "xiaozhi_wifi.h"
#include "LCD\xiaozhi_lcd.h"
#include "LCD\xiaozhi_lvgl.h"

void adc_button_cb(void *button_handle, void *usr_data);
#define TAG "main_button"
typedef enum
{
    SW2_SINGLE_CLICK,
    SW3_SINGLE_CLICK,
    SW3_LONG_PRESS_START
} button_cb;

void app_main(void)
{
    adc_button_init();
    //lcd_app_main();
    adc_button_register_sw2_cb(BUTTON_SINGLE_CLICK, adc_button_cb, (void *)SW2_SINGLE_CLICK);
    adc_button_register_sw3_cb(BUTTON_SINGLE_CLICK, adc_button_cb, (void *)SW3_SINGLE_CLICK);
    adc_button_register_sw3_cb(BUTTON_LONG_PRESS_START, adc_button_cb, (void *)SW3_LONG_PRESS_START);
    xiaozhi_lvgl_init();
    
    //xiaozhi_lvgl_helloworld();
    //xiaozhi_lcd_init();
    //xiaozhi_lcd_test();
    xiaozhi_wifi_init();
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
