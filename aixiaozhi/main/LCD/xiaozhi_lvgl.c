#include "xiaozhi_lvgl.h"

#define TAG "xiaozhi_lvgl"

#define LCD_H_RES 320
#define LCD_V_RES 240
#define LCD_DRAW_BUFF_HEIGHT 10
static lv_display_t *lvgl_disp = NULL;
extern esp_lcd_panel_handle_t panel_handle;
extern esp_lcd_panel_io_handle_t io_handle;
lv_obj_t *qr = NULL;
void xiaozhi_lvgl_init(void)
{
    xiaozhi_lcd_init();
    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,       /* LVGL task priority */
        .task_stack = 4096,       /* LVGL task stack size */
        .task_affinity = -1,      /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = 500, /* Maximum sleep in LVGL task */
        .timer_period_ms = 5      /* LVGL timer tick period in ms */
    };
    lvgl_port_init(&lvgl_cfg);
    // ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

    /* Add LCD screen */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = LCD_H_RES * LCD_DRAW_BUFF_HEIGHT,
        .double_buffer = true,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,

        .color_format = LV_COLOR_FORMAT_RGB565,

        .rotation = {
            .swap_xy = true, // TODO 屏幕旋转
            .mirror_x = true,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = true,

            .swap_bytes = true,
        }};
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);
}

void xiaozhi_lvgl_helloworld(void)
{
    lvgl_port_lock(0);

    /*Change the active screen's background color*/
    lv_obj_t *screenObj = lv_screen_active();
    lv_obj_set_style_bg_color(screenObj, lv_color_hex(0x003a57), LV_PART_MAIN);

    /*Create a white label, set its text and align it to the center*/
    lv_obj_t *label = lv_label_create(screenObj);
    lv_label_set_text(label, "Hello world");
    lv_obj_set_style_text_color(screenObj, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lvgl_port_unlock();
}
void xiaozhi_lvgl_qrcode_show(char *data)
{

    lvgl_port_lock(0);

    lv_color_t bg_color = lv_palette_lighten(LV_PALETTE_LIGHT_BLUE, 5);
    lv_color_t fg_color = lv_palette_darken(LV_PALETTE_BLUE, 4);

    qr = lv_qrcode_create(lv_screen_active());
    lv_qrcode_set_size(qr, 150);
    lv_qrcode_set_dark_color(qr, fg_color);
    lv_qrcode_set_light_color(qr, bg_color);

    lv_obj_set_style_border_color(qr, bg_color, 0);
    lv_obj_set_style_border_width(qr, 5, 0);
    lv_obj_center(qr);

    lv_qrcode_update(qr, data, strlen(data));

    lvgl_port_unlock();
}

void xiaozhi_lvgl_del_qrcode(void)
{
  if (qr == NULL)
  {
    return;
  }
  
  lvgl_port_lock(0);

  lv_obj_del(qr);

  lvgl_port_unlock();

}