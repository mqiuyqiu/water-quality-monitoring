/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    esp8266.c
  * @brief   ESP8266 WiFi 模块驱动（AT 指令，通过 USART2 通信）
  *          功能：AT 自检、连接 WiFi、TCP 连接、HTTP GET 数据上报
  *
  *          使用前提：
  *          1. CubeMX 中 USART2 已开启全局中断（NVIC -> USART2 global interrupt）
  *          2. ESP8266 波特率与 USART2 一致（默认 115200）
  ******************************************************************************
  */
/* USER CODE END Header */

#include "esp8266.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

/* 外部串口句柄 */
extern UART_HandleTypeDef huart2;

/* 接收缓冲区 */
uint8_t esp8266_rx_buf[512];
volatile uint16_t esp8266_rx_len = 0;

/* 单字节接收缓存 */
static uint8_t esp_rx_byte = 0;

/**
  * @brief USART 接收中断回调（HAL 库弱函数覆盖）
  *        仅处理 USART2（ESP8266），USART1 不影响
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    if (esp8266_rx_len < sizeof(esp8266_rx_buf))
    {
      esp8266_rx_buf[esp8266_rx_len++] = esp_rx_byte;
    }
    /* 继续接收下一字节 */
    HAL_UART_Receive_IT(&huart2, &esp_rx_byte, 1);
  }
}

/**
  * @brief 启动 USART2 中断接收，初始化阶段调用一次
  */
void ESP8266_StartReceive(void)
{
  ESP8266_ClearRxBuf();
  HAL_UART_Receive_IT(&huart2, &esp_rx_byte, 1);
}

/**
  * @brief 清空接收缓冲区
  */
void ESP8266_ClearRxBuf(void)
{
  esp8266_rx_len = 0;
  memset(esp8266_rx_buf, 0, sizeof(esp8266_rx_buf));
}

/**
  * @brief 等待指定应答关键字，超时返回
  */
static uint8_t ESP8266_WaitResp(const char *expect, uint32_t timeout_ms)
{
  uint32_t start = HAL_GetTick();

  while (HAL_GetTick() - start < timeout_ms)
  {
    if (esp8266_rx_len > 0)
    {
      /* 命中期望关键字 */
      if (expect != NULL && strstr((char *)esp8266_rx_buf, expect) != NULL)
        return ESP_OK;

      /* AT 报错 */
      if (strstr((char *)esp8266_rx_buf, "ERROR") != NULL ||
          strstr((char *)esp8266_rx_buf, "FAIL") != NULL)
        return ESP_ERROR;
    }
    HAL_Delay(5);
  }
  return ESP_TIMEOUT;
}

/**
  * @brief 发送 AT 指令并等待应答
  * @param cmd      指令内容（含 \r\n）
  * @param expect   期望应答关键字，如 "OK"；不需要判断传 NULL
  * @param timeout_ms 超时时间
  */
uint8_t ESP8266_SendCmd(const char *cmd, const char *expect, uint32_t timeout_ms)
{
  ESP8266_ClearRxBuf();
  HAL_UART_Transmit(&huart2, (uint8_t *)cmd, strlen(cmd), 100);
  return ESP8266_WaitResp(expect, timeout_ms);
}

/**
  * @brief 连接 WiFi（Station 模式）
  * @param ssid 路由器名（不能有引号）
  * @param pwd  路由器密码
  */
uint8_t ESP8266_JoinAP(const char *ssid, const char *pwd)
{
  char cmd[160];

  /* 1. AT 自检 */
  if (ESP8266_SendCmd("AT\r\n", "OK", 2000) != ESP_OK)
    return ESP_ERROR;

  /* 2. 设置为 Station 模式 */
  if (ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK", 2000) != ESP_OK)
    return ESP_ERROR;

  /* 3. 连接路由器，应答 "WIFI GOT IP" 表示成功 */
  sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pwd);
  if (ESP8266_SendCmd(cmd, "WIFI GOT IP", 20000) != ESP_OK)
    return ESP_ERROR;

  return ESP_OK;
}

/**
  * @brief 获取本机 IP
  */
uint8_t ESP8266_GetIP(char *ip_buf, uint32_t timeout_ms)
{
  ESP8266_ClearRxBuf();
  HAL_UART_Transmit(&huart2, (uint8_t *)"AT+CIFSR\r\n", strlen("AT+CIFSR\r\n"), 100);

  uint32_t start = HAL_GetTick();
  while (HAL_GetTick() - start < timeout_ms)
  {
    if (esp8266_rx_len > 0)
    {
      char *p = strstr((char *)esp8266_rx_buf, "STAIP,");
      if (p != NULL)
      {
        sscanf(p, "STAIP,\"%15[^\"]\"", ip_buf);
        return ESP_OK;
      }
    }
    HAL_Delay(5);
  }
  return ESP_TIMEOUT;
}

/**
  * @brief 建立 TCP 连接
  */
uint8_t ESP8266_TCPConnect(const char *host, const char *port)
{
  char cmd[160];
  sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", host, port);
  if (ESP8266_SendCmd(cmd, "CONNECT", 5000) != ESP_OK)
    return ESP_ERROR;
  return ESP_OK;
}

/**
  * @brief TCP 透传发送一段数据
  */
uint8_t ESP8266_SendTCPData(const char *data)
{
  char cmd[32];
  uint32_t len = strlen(data);

  /* 发送长度，等待 ">" 提示符 */
  sprintf(cmd, "AT+CIPSEND=%lu\r\n", (unsigned long)len);
  if (ESP8266_SendCmd(cmd, ">", 3000) != ESP_OK)
    return ESP_ERROR;

  /* 发送数据本体 */
  ESP8266_ClearRxBuf();
  HAL_UART_Transmit(&huart2, (uint8_t *)data, len, 100);

  /* 等待发送完成 */
  if (ESP8266_WaitResp("SEND OK", 5000) != ESP_OK)
    return ESP_ERROR;

  return ESP_OK;
}

/**
  * @brief 关闭 TCP 连接
  */
void ESP8266_TCPClose(void)
{
  ESP8266_SendCmd("AT+CIPCLOSE\r\n", "CLOSED", 3000);
}

/**
  * @brief 巴法云 TCP 数据上报（官方协议，绕开 HTTP 接口限制）
  * @param uid   用户私钥（控制台获取）
  * @param topic 主题名称（仅限字母数字）
  * @param msg   数据内容，自由字符串，如 "7.25,120,25.6"
  * @return ESP_OK 上报成功（收到 cmd=2&res=1）
  *         每次短连接：连上-发送-断开，免费额度下够用
  */
uint8_t ESP8266_ReportBemfa(const char *uid, const char *topic, const char *msg)
{
  char cmd[200];
  uint8_t ret;

  /* 1. 连接巴法云 TCP 服务器（bemfa.com:8344） */
  ret = ESP8266_TCPConnect("bemfa.com", "8344");
  if (ret != ESP_OK)
    return ret;

  /* 2. 构造发布指令：cmd=2&uid=私钥&topic=主题&msg=数据\r\n */
  snprintf(cmd, sizeof(cmd), "cmd=2&uid=%s&topic=%s&msg=%s\r\n", uid, topic, msg);

  /* 3. 发送并等待 res=1 */
  ret = ESP8266_SendTCPData(cmd);
  ESP8266_TCPClose();
  if (ret != ESP_OK)
    return ret;

  return ESP_OK;
}

/**
  * @brief HTTP GET 数据上报
  * @param host 服务器域名，如 "api.bemfa.com"
  * @param port 端口，如 "80"
  * @param path 请求路径（含参数），如 "/api/device/v1/data/1/xxx/7.25?type=1"
  */
uint8_t ESP8266_ReportHTTP(const char *host, const char *port, const char *path)
{
  char req[256];
  uint8_t ret;

  /* 1. 建立 TCP 连接 */
  ret = ESP8266_TCPConnect(host, port);
  if (ret != ESP_OK)
    return ret;

  /* 2. 构造 HTTP GET 请求 */
  snprintf(req, sizeof(req),
           "GET %s HTTP/1.1\r\n"
           "Host: %s\r\n"
           "Connection: close\r\n"
           "\r\n",
           path, host);

  /* 3. 发送请求 */
  ret = ESP8266_SendTCPData(req);
  if (ret != ESP_OK)
  {
    ESP8266_TCPClose();
    return ret;
  }

  /* 4. 关闭连接 */
  ESP8266_TCPClose();
  return ESP_OK;
}
