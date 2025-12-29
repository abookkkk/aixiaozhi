#pragma once
#include "common/common_types.h"
#include "esp_log.h"
#include "esp_websocket_client.h"
#include "esp_crt_bundle.h"
#include "xiaozhi_active.h"
void xiaozhi_ws_init(void);

bool xiaozhi_ws_create_channle(void);

void ws_send_hello(void);
/*发送唤醒消息*/
void xiaozhi_ws_send_wake_word(void);
/*发送开始监听消息*/
void xiaozhi_ws_send_listen(void);
/*发送终止监听消息*/
void xiaozhi_ws_send_stop_listen(void);
/*发送中止消息*/
void xiaozhi_ws_send_stop(void);

void xiaozhi_ws_SendBinary(char *data, int data_len);
/*发送iot的控制命令*/
void xiaozhi_ws_send_iot_command(void);
/*发送iot的状态命令*/
void xiaozhi_ws_send_iot_state(void);