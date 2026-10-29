#include "oled.h"
#include "oledfont.h"
#include "i2c.h"

uint8_t OLED_GRAM[144][8];

/**
 * @brief  屏幕反显控制
 * @param  i: 0=正常显示 1=反色显示
 */
void OLED_ColorTurn(uint8_t i)
{
    if(i == 0)
        OLED_WR_Byte(0xA6, OLED_CMD);
    if(i == 1)
        OLED_WR_Byte(0xA7, OLED_CMD);
}

/**
 * @brief  屏幕180度旋转控制
 * @param  i: 0=正常 1=翻转
 */
void OLED_DisplayTurn(uint8_t i)
{
    if(i == 0)
    {
        OLED_WR_Byte(0xC8, OLED_CMD);
        OLED_WR_Byte(0xA1, OLED_CMD);
    }
    if(i == 1)
    {
        OLED_WR_Byte(0xC0, OLED_CMD);
        OLED_WR_Byte(0xA0, OLED_CMD);
    }
}

/**
 * @brief  底层写一个字节（硬件 I2C 实现，替换原软件模拟）
 * @param  dat: 数据/命令值
 * @param  mode: 0=命令 1=数据
 */
void OLED_WR_Byte(uint8_t dat, uint8_t mode)
{
    uint8_t reg = (mode == OLED_CMD) ? 0x00 : 0x40;
    HAL_I2C_Mem_Write(&hi2c1, OLED_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, &dat, 1, 100);
}

/**
 * @brief  开启OLED显示
 */
void OLED_DisPlay_On(void)
{
    OLED_WR_Byte(0x8D, OLED_CMD);
    OLED_WR_Byte(0x14, OLED_CMD);
    OLED_WR_Byte(0xAF, OLED_CMD);
}

/**
 * @brief  关闭OLED显示
 */
void OLED_DisPlay_Off(void)
{
    OLED_WR_Byte(0x8D, OLED_CMD);
    OLED_WR_Byte(0x10, OLED_CMD);
    OLED_WR_Byte(0xAE, OLED_CMD);
}

/**
 * @brief  将显存数据刷新到屏幕
 */
void OLED_Refresh(void)
{
    uint8_t i, n;
    for(i = 0; i < 8; i++)
    {
        OLED_WR_Byte(0xb0 + i, OLED_CMD);
        OLED_WR_Byte(0x00, OLED_CMD);
        OLED_WR_Byte(0x10, OLED_CMD);
        for(n = 0; n < 128; n++)
            OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA);
    }
}

/**
 * @brief  清屏
 */
void OLED_Clear(void)
{
    uint8_t i, n;
    for(i = 0; i < 8; i++)
    {
        for(n = 0; n < 128; n++)
        {
            OLED_GRAM[n][i] = 0;
        }
    }
    OLED_Refresh();
}

/**
 * @brief  画一个点
 */
void OLED_DrawPoint(uint8_t x, uint8_t y)
{
    uint8_t i, m, n;
    i = y / 8;
    m = y % 8;
    n = 1 << m;
    OLED_GRAM[x][i] |= n;
}

/**
 * @brief  清除一个点
 */
void OLED_ClearPoint(uint8_t x, uint8_t y)
{
    uint8_t i, m, n;
    i = y / 8;
    m = y % 8;
    n = 1 << m;
    OLED_GRAM[x][i] = ~OLED_GRAM[x][i];
    OLED_GRAM[x][i] |= n;
    OLED_GRAM[x][i] = ~OLED_GRAM[x][i];
}

/**
 * @brief  画线
 */
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    uint8_t i, k, k1, k2, y0;
    if((x1 < 0) || (x2 > 128) || (y1 < 0) || (y2 > 64) || (x1 > x2) || (y1 > y2)) return;
    if(x1 == x2)
    {
        for(i = 0; i < (y2 - y1); i++)
        {
            OLED_DrawPoint(x1, y1 + i);
        }
    }
    else if(y1 == y2)
    {
        for(i = 0; i < (x2 - x1); i++)
        {
            OLED_DrawPoint(x1 + i, y1);
        }
    }
    else
    {
        k1 = y2 - y1;
        k2 = x2 - x1;
        k = k1 * 10 / k2;
        for(i = 0; i < (x2 - x1); i++)
        {
            OLED_DrawPoint(x1 + i, y1 + i * k / 10);
        }
    }
}

/**
 * @brief  画圆
 */
void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r)
{
    int a, b, num;
    a = 0;
    b = r;
    while(2 * b * b >= r * r)
    {
        OLED_DrawPoint(x + a, y - b);
        OLED_DrawPoint(x - a, y - b);
        OLED_DrawPoint(x - a, y + b);
        OLED_DrawPoint(x + a, y + b);

        OLED_DrawPoint(x + b, y + a);
        OLED_DrawPoint(x + b, y - a);
        OLED_DrawPoint(x - b, y - a);
        OLED_DrawPoint(x - b, y + a);

        a++;
        num = (a * a + b * b) - r * r;
        if(num > 0)
        {
            b--;
            a--;
        }
    }
}

/**
 * @brief  显示单个字符
 */
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size1)
{
    uint8_t i, m, temp, size2, chr1;
    uint8_t y0 = y;
    size2 = (size1 / 8 + ((size1 % 8) ? 1 : 0)) * (size1 / 2);
    chr1 = chr - ' ';
    for(i = 0; i < size2; i++)
    {
        if(size1 == 12)
            temp = asc2_1206[chr1][i];
        else if(size1 == 16)
            temp = asc2_1608[chr1][i];
        else if(size1 == 24)
            temp = asc2_2412[chr1][i];
        else return;

        for(m = 0; m < 8; m++)
        {
            if(temp & 0x80) OLED_DrawPoint(x, y);
            else OLED_ClearPoint(x, y);
            temp <<= 1;
            y++;
            if((y - y0) == size1)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
}

/**
 * @brief  显示字符串
 */
void OLED_ShowString(uint8_t x, uint8_t y, const char *chr, uint8_t size1)
{
    while((*chr >= ' ') && (*chr <= '~'))
    {
        OLED_ShowChar(x, y, *chr, size1);
        x += size1 / 2;
        if(x > 128 - size1)
        {
            x = 0;
            y += 2;
        }
        chr++;
    }
}

/**
 * @brief  幂运算
 */
uint32_t OLED_Pow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;
    while(n--)
    {
        result *= m;
    }
    return result;
}

/**
 * @brief  显示数字
 */
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size1)
{
    uint8_t t, temp;
    for(t = 0; t < len; t++)
    {
        temp = (num / OLED_Pow(10, len - t - 1)) % 10;
        OLED_ShowChar(x + (size1 / 2) * t, y, temp + '0', size1);
    }
}

/**
 * @brief  显示汉字
 */
void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t num, uint8_t size1)
{
    uint8_t i, m, n = 0, temp, chr1;
    uint8_t x0 = x, y0 = y;
    uint8_t size3 = size1 / 8;
    while(size3--)
    {
        chr1 = num * size1 / 8 + n;
        n++;
        for(i = 0; i < size1; i++)
        {
            if(size1 == 16)
                temp = Hzk1[chr1][i];
            else if(size1 == 24)
                temp = Hzk2[chr1][i];
            else if(size1 == 32)
                temp = Hzk3[chr1][i];
            else if(size1 == 64)
                temp = Hzk4[chr1][i];
            else return;

            for(m = 0; m < 8; m++)
            {
                if(temp & 0x01) OLED_DrawPoint(x, y);
                else OLED_ClearPoint(x, y);
                temp >>= 1;
                y++;
            }
            x++;
            if((x - x0) == size1)
            {
                x = x0;
                y0 = y0 + 8;
            }
            y = y0;
        }
    }
}

/**
 * @brief  滚动显示汉字
 * @note  该函数为死循环，调用后程序会停在此处循环滚动
 */
void OLED_ScrollDisplay(uint8_t num, uint8_t space)
{
    uint8_t i, n, t = 0, m = 0, r;
    while(1)
    {
        if(m == 0)
        {
            OLED_ShowChinese(128, 24, t, 16);
            t++;
        }
        if(t == num)
        {
            for(r = 0; r < 16 * space; r++)
            {
                for(i = 0; i < 144; i++)
                {
                    for(n = 0; n < 8; n++)
                    {
                        OLED_GRAM[i - 1][n] = OLED_GRAM[i][n];
                    }
                }
                OLED_Refresh();
            }
            t = 0;
        }
        m++;
        if(m == 16) { m = 0; }
        for(i = 0; i < 144; i++)
        {
            for(n = 0; n < 8; n++)
            {
                OLED_GRAM[i - 1][n] = OLED_GRAM[i][n];
            }
        }
        OLED_Refresh();
    }
}

/**
 * @brief  设置数据写入起始坐标
 */
void OLED_WR_BP(uint8_t x, uint8_t y)
{
    OLED_WR_Byte(0xb0 + y, OLED_CMD);
    OLED_WR_Byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD);
    OLED_WR_Byte((x & 0x0f) | 0x01, OLED_CMD);
}

/**
 * @brief  显示图片
 * @param  x0,y0: 起始坐标
 * @param  x1,y1: 结束坐标
 * @param  BMP: 图片点阵数组
 */
void OLED_ShowPicture(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t BMP[])
{
    uint32_t j = 0;
    uint8_t x = 0, y = 0;
    if(y % 8 == 0) y = 0;
    else y += 1;
    for(y = y0; y < y1; y++)
    {
        OLED_WR_BP(x0, y);
        for(x = x0; x < x1; x++)
        {
            OLED_WR_Byte(BMP[j], OLED_DATA);
            j++;
        }
    }
}

/**
 * @brief  OLED初始化（硬件I2C版，无需手动初始化GPIO）
 */
void OLED_Init(void)
{
    HAL_Delay(100);  // 上电等待稳定

    OLED_WR_Byte(0xAE, OLED_CMD);  // 关闭显示
    OLED_WR_Byte(0x00, OLED_CMD);  // 设置低列地址
    OLED_WR_Byte(0x10, OLED_CMD);  // 设置高列地址
    OLED_WR_Byte(0x40, OLED_CMD);  // 设置起始行
    OLED_WR_Byte(0x81, OLED_CMD);  // 对比度控制
    OLED_WR_Byte(0xCF, OLED_CMD);  // 对比度值
    OLED_WR_Byte(0xA1, OLED_CMD);  // 段重定向
    OLED_WR_Byte(0xC8, OLED_CMD);  // 扫描方向
    OLED_WR_Byte(0xA6, OLED_CMD);  // 正常显示
    OLED_WR_Byte(0xA8, OLED_CMD);  // 多路复用率
    OLED_WR_Byte(0x3f, OLED_CMD);  // 1/64 占空比
    OLED_WR_Byte(0xD3, OLED_CMD);  // 显示偏移
    OLED_WR_Byte(0x00, OLED_CMD);  // 无偏移
    OLED_WR_Byte(0xd5, OLED_CMD);  // 显示时钟分频
    OLED_WR_Byte(0x80, OLED_CMD);  // 分频系数
    OLED_WR_Byte(0xD9, OLED_CMD);  // 预充电周期
    OLED_WR_Byte(0xF1, OLED_CMD);  // 预充电值
    OLED_WR_Byte(0xDA, OLED_CMD);  // COM引脚硬件配置
    OLED_WR_Byte(0x12, OLED_CMD);
    OLED_WR_Byte(0xDB, OLED_CMD);  // VCOMH电压
    OLED_WR_Byte(0x40, OLED_CMD);
    OLED_WR_Byte(0x20, OLED_CMD);  // 内存地址模式
    OLED_WR_Byte(0x02, OLED_CMD);  // 页地址模式
    OLED_WR_Byte(0x8D, OLED_CMD);  // 电荷泵设置
    OLED_WR_Byte(0x14, OLED_CMD);  // 开启电荷泵
    OLED_WR_Byte(0xA4, OLED_CMD);  // 全局显示开启
    OLED_WR_Byte(0xA6, OLED_CMD);  // 正常显示
    OLED_WR_Byte(0xAF, OLED_CMD);  // 开启显示

    OLED_Clear();
}