#include "xiaozhi_audio.h"

static i2c_master_bus_handle_t i2c0_bus_handle;

i2s_chan_handle_t i2s_tx_handle;
i2s_chan_handle_t i2s_rx_handle;
esp_codec_dev_handle_t play_dev;

void xiaozhi_audio_i2c_init(void)
{
    i2c_master_bus_config_t i2c_bus_config = {0};
    i2c_bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
    i2c_bus_config.i2c_port = I2C_NUM_0;
    i2c_bus_config.scl_io_num = GPIO_NUM_1;
    i2c_bus_config.sda_io_num = GPIO_NUM_0;
    i2c_bus_config.glitch_ignore_cnt = 7;
    i2c_bus_config.flags.enable_internal_pullup = true;
    i2c_new_master_bus(&i2c_bus_config, &i2c0_bus_handle);
}

void xiaozhi_audio_i2s_init(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(16, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = GPIO_NUM_3,
            .bclk = GPIO_NUM_2,
            .ws = GPIO_NUM_5,
            .dout = GPIO_NUM_6,
            .din = GPIO_NUM_4,
        },
    };
    // 生成i2s通道配置
    i2s_new_channel(&chan_cfg, &i2s_tx_handle, &i2s_rx_handle);
    // 初始化收发模式
    i2s_channel_init_std_mode(i2s_tx_handle,&std_cfg);
    i2s_channel_init_std_mode(i2s_rx_handle,&std_cfg);
    // 使能收发模式
    i2s_channel_enable(i2s_tx_handle);
    i2s_channel_enable(i2s_rx_handle);
}

void xiaozhi_audio_init(void)
{
    // 1.初始化i2c和i2s接口
    xiaozhi_audio_i2c_init();
    xiaozhi_audio_i2s_init();

    // 对相关接口初始化
    audio_codec_i2s_cfg_t i2s_cfg = {
        .rx_handle = i2s_rx_handle,
        .tx_handle = i2s_tx_handle,
    };

    const audio_codec_data_if_t *data_if = audio_codec_new_i2s_data(&i2s_cfg);
    

    // 2.初始化音频接口
    audio_codec_i2c_cfg_t i2c_cfg = {.addr = ES8311_CODEC_DEFAULT_ADDR,
                                     .bus_handle = i2c0_bus_handle};

    const audio_codec_ctrl_if_t *out_ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg);
    const audio_codec_gpio_if_t *gpio_if = audio_codec_new_gpio();
    // 3.实现""audio_codec_if_t"接口
    es8311_codec_cfg_t es8311_cfg = {
        .codec_mode = ESP_CODEC_DEV_WORK_MODE_DAC,
        .ctrl_if = out_ctrl_if,
        .gpio_if = gpio_if,
        .pa_pin = GPIO_NUM_7,
        .use_mclk = true,
    };
    const audio_codec_if_t *out_codec_if = es8311_codec_new(&es8311_cfg);
    // 4.
    esp_codec_dev_cfg_t dev_cfg = {
        .codec_if = out_codec_if,
        .data_if = data_if,
        .dev_type = ESP_CODEC_DEV_TYPE_IN_OUT,
    };
    /*音频设备的句柄，我们的播放和录音都是通过这个句柄来操作的*/
    play_dev = esp_codec_dev_new(&dev_cfg);
    // 5.播放音频
    esp_codec_dev_set_out_vol(play_dev, 80.0);
    esp_codec_dev_set_in_gain(play_dev, 15.0);
    esp_codec_dev_sample_info_t fs = {
        .sample_rate = 16000,//采样率
        .channel = 1,       //单声道
        .bits_per_sample = 16,//16位宽
    };
    esp_codec_dev_open(play_dev, &fs);
}

void xiaozhi_audio_play(void *buf, uint32_t size)
{
    esp_codec_dev_write(play_dev, buf, size);
}
void xiaozhi_audio_record(void *buf, uint32_t size)
{
    // 录音
    esp_codec_dev_read(play_dev, buf, size);
}
