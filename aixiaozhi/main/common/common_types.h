#pragma once
#include "stdbool.h"
#include "esp_wn_iface.h"
#include "esp_vad.h"

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
  /*小智的状态改变回调函数*/
  void (*xiaozhi_vad_changed_callback)(void);
  /*小智的唤醒回调函数*/
  void (*xiaozhi_wakeup_callback)(void);
} XiaoZhi_Handle_t;



extern XiaoZhi_Handle_t xiaozhi_handle;