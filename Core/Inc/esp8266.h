#ifndef __ESP8266_H
#define __ESP8266_H

#include "main.h"

/* 返回状态 */
#define ESP_OK       0
#define ESP_TIMEOUT  1
#define ESP_ERROR    2

/* 接收缓冲区（中断接收，USART2） */
extern uint8_t esp8266_rx_buf[512];
extern volatile uint16_t esp8266_rx_len;

/* 启动 USART2 中断接收（放在初始化阶段调用一次） */
void ESP8266_StartReceive(void);

/* 清空接收缓冲区 */
void ESP8266_ClearRxBuf(void);

/* 发送 AT 指令并等待应答，expect 为期望返回关键字，如 "OK" */
uint8_t ESP8266_SendCmd(const char *cmd, const char *expect, uint32_t timeout_ms);

/* 连接 WiFi（Station 模式），ssid/pwd 为你的路由器账号密码 */
uint8_t ESP8266_JoinAP(const char *ssid, const char *pwd);

/* 获取本机 IP（填入 ip_buf） */
uint8_t ESP8266_GetIP(char *ip_buf, uint32_t timeout_ms);

/* 建立 TCP 连接 */
uint8_t ESP8266_TCPConnect(const char *host, const char *port);

/* TCP 透传发送数据 */
uint8_t ESP8266_SendTCPData(const char *data);

/* 关闭 TCP 连接 */
void ESP8266_TCPClose(void);

/* HTTP GET 上报（云平台数据接口） */
uint8_t ESP8266_ReportHTTP(const char *host, const char *port, const char *path);

#endif
