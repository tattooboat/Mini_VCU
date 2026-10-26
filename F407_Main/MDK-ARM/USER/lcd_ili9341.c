#include "lcd_ili9341.h"

/* 软件 SPI 引脚 */
#define LCD_SCK_PORT    GPIOC
#define LCD_SCK_PIN     GPIO_PIN_0

#define LCD_MOSI_PORT   GPIOC
#define LCD_MOSI_PIN    GPIO_PIN_1

#define LCD_CS_PORT     GPIOC
#define LCD_CS_PIN      GPIO_PIN_3

#define LCD_DC_PORT     GPIOC
#define LCD_DC_PIN      GPIO_PIN_4

#define LCD_RST_PORT    GPIOC
#define LCD_RST_PIN     GPIO_PIN_5

static void LCD_Delay(volatile uint32_t n)
{
    while (n--) {
        __NOP();
    }
}

static void LCD_WriteByte(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(
            LCD_SCK_PORT, LCD_SCK_PIN, GPIO_PIN_RESET);

        HAL_GPIO_WritePin(
            LCD_MOSI_PORT, LCD_MOSI_PIN,
            (data & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);

        LCD_Delay(10);

        HAL_GPIO_WritePin(
            LCD_SCK_PORT, LCD_SCK_PIN, GPIO_PIN_SET);

        LCD_Delay(10);

        data <<= 1;
    }

    HAL_GPIO_WritePin(
        LCD_SCK_PORT, LCD_SCK_PIN, GPIO_PIN_RESET);
}

static void LCD_WriteCommand(uint8_t cmd)
{
    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET);

    LCD_WriteByte(cmd);

    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

static void LCD_WriteData(uint8_t data)
{
    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);

    LCD_WriteByte(data);

    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

static void LCD_WriteRegister(uint8_t cmd,
                              const uint8_t *data,
                              uint8_t len)
{
    LCD_WriteCommand(cmd);

    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);

    for (uint8_t i = 0; i < len; i++) {
        LCD_WriteByte(data[i]);
    }

    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

static void LCD_SetWindow(uint16_t x0, uint16_t y0,
                          uint16_t x1, uint16_t y1)
{
    uint8_t d[4];

    d[0] = x0 >> 8;
    d[1] = x0 & 0xFF;
    d[2] = x1 >> 8;
    d[3] = x1 & 0xFF;
    LCD_WriteRegister(0x2A, d, 4);  // Column Address Set

    d[0] = y0 >> 8;
    d[1] = y0 & 0xFF;
    d[2] = y1 >> 8;
    d[3] = y1 & 0xFF;
    LCD_WriteRegister(0x2B, d, 4);  // Page Address Set

    LCD_WriteCommand(0x2C);         // Memory Write
}

void LCD_Init(void)
{
    const uint8_t pixfmt[] = {0x55}; // RGB565

    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LCD_SCK_PORT, LCD_SCK_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_MOSI_PORT, LCD_MOSI_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);

    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(120);

    LCD_WriteCommand(0x01);  // Software Reset
    HAL_Delay(120);

    LCD_WriteCommand(0x11);  // Sleep Out
    HAL_Delay(120);

    LCD_WriteRegister(0x3A, pixfmt, 1); // 16-bit RGB565

    /* MADCTL: 初始方向，后续可根据实际面板调整 */
    {
        const uint8_t madctl[] = {0x28};
        LCD_WriteRegister(0x36, madctl, 1);
    }

    LCD_WriteCommand(0x29);  // Display ON
    HAL_Delay(20);
}

void LCD_FillColor(uint16_t color)
{
    uint32_t total = (uint32_t)LCD_WIDTH * LCD_HEIGHT;
    uint8_t high = color >> 8;
    uint8_t low  = color & 0xFF;

    LCD_SetWindow(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);

    for (uint32_t i = 0; i < total; i++) {
        LCD_WriteByte(high);
        LCD_WriteByte(low);
    }

    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}
