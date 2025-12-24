#include "xiaozhi_lvgl.h"

#define TAG "xiaozhi_lvgl"

#define LCD_H_RES 320
#define LCD_V_RES 240
#define LCD_DRAW_BUFF_HEIGHT 10
static lv_display_t *lvgl_disp = NULL;
extern esp_lcd_panel_handle_t panel_handle;
extern esp_lcd_panel_io_handle_t io_handle;

lv_obj_t *qr = NULL;
lv_obj_t *screenObj;
lv_obj_t *title_obj;
lv_obj_t *emoji_label;
lv_obj_t *center_text;
lv_obj_t *title_label;
void xiaozhi_lvgl_text_init(void);

typedef struct
{
    char *name;
    char *emoji;
} emoji_list_t;
const emoji_list_t emoji_list[21] = {
    {.name = "neutral", .emoji = "😶"},
    {.name = "happy", .emoji = "🙂"},
    {.name = "laughing", .emoji = "😆"},
    {.name = "funny", .emoji = "😂"},
    {.name = "sad", .emoji = "😔"},
    {.name = "angry", .emoji = "😠"},
    {.name = "crying", .emoji = "😭"},
    {.name = "loving", .emoji = "😍"},
    {.name = "embarrassed", .emoji = "😳"},
    {.name = "surprised", .emoji = "😲"},
    {.name = "shocked", .emoji = "😱"},
    {.name = "thinking", .emoji = "🤔"},
    {.name = "winking", .emoji = "😉"},
    {.name = "cool", .emoji = "😎"},
    {.name = "relaxed", .emoji = "😌"},
    {.name = "delicious", .emoji = "🤤"},
    {.name = "kissy", .emoji = "😘"},
    {.name = "confident", .emoji = "😏"},
    {.name = "sleepy", .emoji = "😴"},
    {.name = "silly", .emoji = "😜"},
    {.name = "confused", .emoji = "🙄"}};
void xiaozhi_lvgl_init(void)
{
    xiaozhi_lcd_init();
    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,       /* LVGL task priority */
        .task_stack = 8192,       /* LVGL task stack size */
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

    //! 屏幕的初始化
    lvgl_port_lock(0);
    screenObj = lv_screen_active();
    /*设置屏幕的背景颜色*/
    lv_obj_set_style_bg_color(screenObj, lv_color_hex(0xffffff), LV_PART_MAIN);
    /*设置屏幕的文字颜色*/
    lv_obj_set_style_text_color(screenObj, lv_color_hex(0x000000), LV_PART_MAIN);
    lvgl_port_unlock();
    xiaozhi_lvgl_text_init();
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

void xiaozhi_lvgl_text_init()
{
    lvgl_port_lock(0);
    //! title obj
    title_obj = lv_obj_create(screenObj);
    lv_obj_set_size(title_obj, 320, 30);
    /*设置为无边框*/
    lv_obj_set_style_border_width(title_obj, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(title_obj, 0, LV_PART_MAIN);
    /*设置背景颜色*/
    lv_obj_set_style_bg_color(title_obj, lv_color_hex(0x99a2a8), LV_PART_MAIN);
    /*创建文本标签*/
    title_label = lv_label_create(title_obj);
    lv_label_set_text(title_label, "没有网络连接，请复位！！");

    /*将title_obj设置为flex布局*/
    lv_obj_set_layout(title_obj, LV_LAYOUT_FLEX);
    /*去掉title的滚动条*/
    lv_obj_set_scrollbar_mode(title_obj,LV_SCROLLBAR_MODE_OFF);
    /*设置水平布局*/
    lv_obj_set_flex_flow(title_obj, LV_FLEX_FLOW_ROW);
    /*内部文字居中*/
    lv_obj_set_flex_align(title_obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    /*声明使用字体*/
    lv_obj_set_style_text_font(title_label, &font_puhui_20_4, LV_PART_MAIN);

    //lv_label_set_long_mode(title_label, LV_LABEL_LONG_SCROLL_CIRCULAR); // 循环滚动模式

    //! emoji 表情
    emoji_label = lv_label_create(screenObj);
    /*设置emoji_label 宽度为320*/
    lv_obj_set_width(emoji_label, 320);
    /*设置文本为水平居中*/
    lv_obj_set_style_text_align(emoji_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    /*设置为相对title_obj下方居中*/
    lv_obj_align_to(emoji_label, title_obj, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    /*声明使用字体*/
    lv_obj_set_style_text_font(emoji_label, font_emoji_64_init(), LV_PART_MAIN);
    lv_label_set_text(emoji_label, "🙂");

    //! center_text
    center_text = lv_label_create(screenObj);
    /*设置center_text 宽度为300*/
    lv_obj_set_width(center_text, 300);
    /*设置文本水平居中*/
    lv_obj_set_style_text_align(center_text, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    /*设置center_text 相对于title_obj居中*/
    lv_obj_align_to(center_text, emoji_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    /*声明使用字体*/
    lv_obj_set_style_text_font(center_text, &font_puhui_20_4, LV_PART_MAIN);
    lv_label_set_text(center_text, "你好，我是小智!");
    /*设置长文本滚动*/
    lv_label_set_long_mode(center_text, LV_LABEL_LONG_SCROLL_CIRCULAR); // 循环滚动模式
    lvgl_port_unlock();
}

void xiaozhi_lvgl_title_label_update(char *text)
{
    lvgl_port_lock(0);
    lv_label_set_text(title_label, text);
    lvgl_port_unlock();
}

void xiaozhi_lvgl_emoji_label_update(char *emoji)
{
    lvgl_port_lock(0);
    bool isFind = false;
    for (uint8_t i = 0; i < 21; i++)
    {
        if (strcmp(emoji_list[i].name, emoji) == 0)
        {
            lv_label_set_text(emoji_label, emoji_list[i].emoji);
            isFind = true;
        }
    }
    if (!isFind)
    {
        lv_label_set_text(emoji_label, "happy");
    }

    lvgl_port_unlock();
}

void xiaozhi_lvgl_center_text_update(char *text)
{
    lvgl_port_lock(0);
    lv_label_set_text(center_text, text);
    lvgl_port_unlock();
}