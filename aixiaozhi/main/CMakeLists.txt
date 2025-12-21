idf_component_register(
    SRCS 
    "main.c"
    "adc_button/adc_button.c"
    "connect/xiaozhi_wifi.c"
    "LCD/xiaozhi_lcd.c"
    "LCD/xiaozhi_lvgl.c"
    INCLUDE_DIRS 
    "."
    "adc_button"
    "connect"
    "LCD"
    EMBED_FILES image.jpg


)
