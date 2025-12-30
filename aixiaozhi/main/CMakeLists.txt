idf_component_register(
    SRCS 
    "main.c"
    "adc_button/adc_button.c"
    "connect/xiaozhi_wifi.c"
    "connect/xiaozhi_active.c"
    "connect/xiaozhi_ws.c"
    "LCD/xiaozhi_lcd.c"
    "LCD/xiaozhi_lvgl.c"
    "audio/xiaozhi_audio.c"
    "audio/xiaozhi_sr.c"
    "audio/xiaozhi_audio_codec.c"
    "audio/xiaozhi_audio_decode.c"

    INCLUDE_DIRS 
    "."
    "adc_button"
    "connect"
    "LCD"
    "audio"
    "iot_data"
    # EMBED_FILES image.jpg


)
