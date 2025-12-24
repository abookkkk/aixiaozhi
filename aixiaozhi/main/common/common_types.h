#pragma once
#include "stdbool.h"


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


} XiaoZhi_Handle_t;



extern XiaoZhi_Handle_t xiaozhi_handle;