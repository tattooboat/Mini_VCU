#include "Int_XPT2046.h"

volatile uint16_t g_touch_raw_x = 0;
volatile uint16_t g_touch_raw_y = 0;

/*
 * STM32F407VGT6
 * T_CLK  -> PB5
 * T_CS   -> PB15
 * T_DIN  -> PB4
 * T_DO   -> PB3
 *
 * GPIO 模拟 SPI，MSB first
 */

#define TP_CLK_PORT    GPIOB
#define TP_CLK_PIN     GPIO_PIN_5

#define TP_CS_PORT     GPIOB
#define TP_CS_PIN      GPIO_PIN_15

#define TP_DIN_PORT    GPIOB
#define TP_DIN_PIN     GPIO_PIN_4

#define TP_DO_PORT     GPIOB
#define TP_DO_PIN      GPIO_PIN_3


/* 短延时：先使用保守的低速时序 */
static void TP_Delay(void)
{
    volatile uint32_t i;

    for (i = 0; i < 20U; i++)
    {
        __NOP();
    }
}

static void TP_WriteBit(uint8_t bit)
{
    HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(
        TP_DIN_PORT,
        TP_DIN_PIN,
        bit ? GPIO_PIN_SET : GPIO_PIN_RESET
    );

    TP_Delay();

    HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_SET);
    TP_Delay();
}

static uint8_t TP_ReadBit(void)
{
    uint8_t bit;

    HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_RESET);
    TP_Delay();

    bit = (HAL_GPIO_ReadPin(TP_DO_PORT, TP_DO_PIN)
           == GPIO_PIN_SET) ? 1U : 0U;

    HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_SET);
    TP_Delay();

    return bit;
}

/* 发送 8 位命令 */
static void TP_WriteByte(uint8_t data)
{
    uint8_t i;

    for (i = 0; i < 8U; i++)
    {
        TP_WriteBit((data & 0x80U) != 0U);
        data <<= 1;
    }
}

/* 读取 XPT2046 的 12 位 ADC 数据 */
static uint16_t TP_ReadADC(uint8_t command)
{
    uint8_t i;
    uint16_t value = 0U;

    HAL_GPIO_WritePin(TP_CS_PORT, TP_CS_PIN, GPIO_PIN_RESET);
    TP_Delay();

    TP_WriteByte(command);

    /*
     * 读取 16 个时钟位。
     * 常见 XPT2046 时序中，12 位 ADC 数据位于
     * 接收数据的高 12 位。
     */
    for (i = 0; i < 16U; i++)
    {
        value <<= 1;
        value |= TP_ReadBit();
    }

    HAL_GPIO_WritePin(TP_CS_PORT, TP_CS_PIN, GPIO_PIN_SET);

    return (value >> 3) & 0x0FFFU;
}

void Int_XPT2046_Init(void)
{
    HAL_GPIO_WritePin(TP_CS_PORT, TP_CS_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TP_DIN_PORT, TP_DIN_PIN, GPIO_PIN_RESET);
}

// 读取一次原始 X/Y 坐标，返回 1 表示读取完成
uint8_t Int_XPT2046_ReadRaw(uint16_t *x, uint16_t *y)
{
    if ((x == NULL) || (y == NULL))
    {
        return 0U;
    }

    /*
     * XPT2046 常用命令：
     * 0xD0：读取 X
     * 0x90：读取 Y
     *
     * 此阶段只读取原始值，不做旋转和校准。
     */
    *x = TP_ReadADC(0xD0U);
    *y = TP_ReadADC(0x90U);

    return 1U;
}

/* 判断是否有触摸压力 */
uint8_t Int_XPT2046_IsPressed(void)
{
    uint16_t z1;
    uint16_t z2;

    /*
     * XPT2046:
     * 0xB0：读取 Z1
     * 0xC0：读取 Z2
     */
    z1 = TP_ReadADC(0xB0U);
    z2 = TP_ReadADC(0xC0U);

    /*
     * 初步判断：
     * Z1、Z2 都不能接近 0 或满量程。
     * 具体阈值需要根据实测调整。
     */
    if ((z1 > 100U) &&
        (z1 < 4000U) &&
        (z2 > 100U) &&
        (z2 < 4000U))
    {
        return 1U;
    }

    return 0U;
}
