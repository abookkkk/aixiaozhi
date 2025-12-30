#pragma once
#include "stdbool.h"
#include "esp_wn_iface.h"
#include "esp_vad.h"
#include "freertos/ringbuf.h"
/*小智的状态*/
typedef enum
{
  XIAOZHI_STATE_IDLE = 0,
  XIAOZHI_STATE_LISTENING,
  XIAOZHI_STATE_SPEAKING,
} XiaoZhi_State_t;

/* 定义小智的句柄 */
typedef struct

{
  /* websokcet_url */
  char *websocket_url;
  /* websocket_token */
  char *websocket_token;
  /* 设备激活码 */
  char *active_code;
  /* 激活的状态 */
  bool is_actived;
  /* 唤醒状态 */
  bool is_wakeup;
  /*当前状态*/
  vad_state_t current_vad_state;
  /*上一次的状态*/
  vad_state_t last_vad_state;
  /*从sr到coder的环形缓冲区*/
  RingbufHandle_t sr2coder_ringbuf_handle;
  /*从coder到ws的环形缓冲区*/
  RingbufHandle_t coder2ws_ringbuf_handle;
  /*从ws到解码器的环形缓冲区*/
  RingbufHandle_t ws2decoder_ringbuf_handle;
  /*小智的状态改变回调函数*/
  void (*xiaozhi_vad_changed_callback)(void);
  /*小智的唤醒回调函数*/
  void (*xiaozhi_wakeup_callback)(void);
  /* 处理websocket服务器返回的文本数据的回调函数 */
  void (*ws_text_proccess)(char *text, int text_len);
  /* 处理websocket服务器返回的二进制数据的回调函数 */
  void (*ws_binary_proccess)(uint8_t *data, int data_len);

  /*小智的事件组*/
  EventGroupHandle_t xiaozhi_event_group;
  /*小智的状态*/
  XiaoZhi_State_t xiaozhi_state;
} XiaoZhi_Handle_t;

/* 定义是否链接websocket服务器成功的标志位 */
#define WS_CONNECT_SUCCESS_BIT BIT0
/* 定义是否是HELLO的回应标志位 */
#define WS_HELLO_RESPONSE_BIT BIT1
/*ai的名字*/
#define AI_NAME "宝儿"
extern XiaoZhi_Handle_t xiaozhi_handle;

