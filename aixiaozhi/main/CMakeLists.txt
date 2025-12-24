idf_component_register(
    SRCS 
    "main.c"
    "adc_button/adc_button.c"
    "connect/xiaozhi_wifi.c"
    "connect/xiaozhi_active.c"
    "LCD/xiaozhi_lcd.c"
    "LCD/xiaozhi_lvgl.c"
    "audio/xiaozhi_audio.c"
    "audio/xiaozhi_sr.c"

    INCLUDE_DIRS 
    "."
    "adc_button"
    "connect"
    "LCD"
    "audio"
    EMBED_FILES image.jpg


)
