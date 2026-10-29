#ifndef __OLED_H
#define __OLED_H
#include "main.h"
#include "stdlib.h"

#define OLED_CMD  0    // 写命令标识
#define OLED_DATA 1    // 写数据标识
#define OLED_ADDRESS 0x78  // I2C 8位地址，屏幕不亮可改为 0x7A

/* 函数声明 */
void OLED_ColorTurn(uint8_t i);
void OLED_DisplayTurn(uint8_t i);
void OLED_WR_Byte(uint8_t dat, uint8_t mode);
void OLED_DisPlay_On(void);
void OLED_DisPlay_Off(void);
void OLED_Refresh(void);
void OLED_Clear(void);
void OLED_DrawPoint(uint8_t x, uint8_t y);
void OLED_ClearPoint(uint8_t x, uint8_t y);
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r);
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size1);
void OLED_ShowString(uint8_t x, uint8_t y, const char *chr, uint8_t size1);
uint32_t OLED_Pow(uint8_t m, uint8_t n);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size1);
void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t num, uint8_t size1);
void OLED_ScrollDisplay(uint8_t num, uint8_t space);
void OLED_WR_BP(uint8_t x, uint8_t y);
void OLED_ShowPicture(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t BMP[]);
void OLED_Init(void);

#endif