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

    /* 电源与显示框设置（ILI9341 通用初始化序列） */
    {
        const uint8_t d[] = {0x00, 0xC1, 0x30};
        LCD_WriteRegister(0xCF, d, 3); // Power Control B

        const uint8_t e[] = {0x64, 0x03, 0x12, 0x81};
        LCD_WriteRegister(0xED, e, 4); // Power ON Sequence

        const uint8_t f[] = {0x85, 0x00, 0x78};
        LCD_WriteRegister(0xE8, f, 3); // Driver Timing Control A

        const uint8_t g[] = {0x39, 0x2C, 0x00, 0x34, 0x02};
        LCD_WriteRegister(0xCB, g, 5); // Power Control A

        const uint8_t h[] = {0x20};
        LCD_WriteRegister(0xF7, h, 1); // Pump Ratio Control

        const uint8_t i[] = {0x00, 0x00};
        LCD_WriteRegister(0xEA, i, 2); // Driver Timing Control B

        const uint8_t j[] = {0x23};
        LCD_WriteRegister(0xC0, j, 1); // Power Control 1 (VRH)

        const uint8_t k[] = {0x10};
        LCD_WriteRegister(0xC1, k, 1); // Power Control 2 (SAP)

        const uint8_t l[] = {0x3E, 0x28};
        LCD_WriteRegister(0xC5, l, 2); // VCM Control

        const uint8_t m[] = {0x86};
        LCD_WriteRegister(0xC7, m, 1); // VCM Control 2
    }

    /* 显示功能控制：关闭部分显示、固定扫描方向（修复只能显示部分区域的经典问题） */
    {
        const uint8_t d[] = {0x08, 0x82, 0x27};
        LCD_WriteRegister(0xB6, d, 3);

        const uint8_t e[] = {0x00, 0x18};
        LCD_WriteRegister(0xB1, e, 2); // Frame Rate

        const uint8_t f[] = {0x00};
        LCD_WriteRegister(0xF2, f, 1); // 3-Gamma Control Disable

        const uint8_t g[] = {0x01};
        LCD_WriteRegister(0x26, g, 1); // Gamma Set
    }

    /* MADCTL: 0x28（MV+BGR）。之前确认方向正确（红绿蓝黄位置对），先固定下来做运行诊断 */
    {
        const uint8_t madctl[] = {0x28};
        LCD_WriteRegister(0x36, madctl, 1);
    }

    /* Gamma 曲线（通用默认值，兼容多数面板） */
    {
        const uint8_t p0[] =
        {
            0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08,
            0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03,
            0x0E, 0x09, 0x00
        };
        LCD_WriteRegister(0xE0, p0, 15); // Positive Gamma

        const uint8_t p1[] =
        {
            0x00, 0x0E, 0x14, 0x03, 0x11, 0x07,
            0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C,
            0x31, 0x36, 0x0F
        };
        LCD_WriteRegister(0xE1, p1, 15); // Negative Gamma
    }

    LCD_WriteCommand(0x29);  // Display ON
    HAL_Delay(120);
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


/**
 * @brief  在指定坐标绘制一个像素点
 */
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    uint8_t high;
    uint8_t low;

    if (x >= LCD_WIDTH || y >= LCD_HEIGHT)
    {
        return;
    }

    high = (uint8_t)(color >> 8);
    low  = (uint8_t)(color & 0xFF);

    LCD_SetWindow(x, y, x, y);

    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);

    LCD_WriteByte(high);
    LCD_WriteByte(low);

    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}


/**
 * @brief  填充矩形区域
 * @note   x、y 为左上角坐标，width、height 为宽高
 */
void LCD_FillRect(uint16_t x, uint16_t y,
                  uint16_t width, uint16_t height,
                  uint16_t color)
{
    uint32_t total;
    uint8_t high;
    uint8_t low;
    uint32_t x_end;
    uint32_t y_end;

    if (width == 0 || height == 0)
    {
        return;
    }

    if (x >= LCD_WIDTH || y >= LCD_HEIGHT)
    {
        return;
    }

    /* 防止矩形超出屏幕边界 */
    x_end = (uint32_t)x + width;
    y_end = (uint32_t)y + height;

    if (x_end > LCD_WIDTH)
    {
        x_end = LCD_WIDTH;
    }

    if (y_end > LCD_HEIGHT)
    {
        y_end = LCD_HEIGHT;
    }

    width  = (uint16_t)(x_end - x);
    height = (uint16_t)(y_end - y);

    total = (uint32_t)width * height;

    high = (uint8_t)(color >> 8);
    low  = (uint8_t)(color & 0xFF);

    LCD_SetWindow(x, y,
                  (uint16_t)(x_end - 1),
                  (uint16_t)(y_end - 1));

    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);

    while (total--)
    {
        LCD_WriteByte(high);
        LCD_WriteByte(low);
    }

    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}


/**
 * @brief  将 RGB565 像素数组写入 ILI9341
 * @note   pixels 按行排列，每个像素占 2 字节
 */
void LCD_DrawRGB565(uint16_t x, uint16_t y,
                    uint16_t width, uint16_t height,
                    const uint16_t *pixels)
{
    uint32_t total;
    uint32_t i;
    uint16_t color;

    if (pixels == NULL || width == 0 || height == 0)
    {
        return;
    }

    /* 确保整个区域都在屏幕范围内 */
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT)
    {
        return;
    }

    if ((uint32_t)x + width > LCD_WIDTH ||
        (uint32_t)y + height > LCD_HEIGHT)
    {
        return;
    }

    total = (uint32_t)width * height;

    /* 设置要更新的矩形区域 */
    LCD_SetWindow(x, y,
                  (uint16_t)(x + width - 1),
                  (uint16_t)(y + height - 1));

    /* 连续发送像素数据 */
    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);

    for (i = 0; i < total; i++)
    {
        color = pixels[i];

        /* ILI9341 当前使用 RGB565，高字节先发 */
        LCD_WriteByte((uint8_t)(color >> 8));
        LCD_WriteByte((uint8_t)(color & 0xFF));
    }

    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

/**
 * @brief  绘制矩形边框
 */
void LCD_DrawRect(uint16_t x, uint16_t y,
                  uint16_t width, uint16_t height,
                  uint16_t color)
{
    if (width == 0 || height == 0)
    {
        return;
    }

    LCD_FillRect(x, y, width, 1, color);

    if (height > 1)
    {
        LCD_FillRect(x, (uint16_t)(y + height - 1),
                     width, 1, color);
    }

    if (height > 2)
    {
        LCD_FillRect(x, (uint16_t)(y + 1),
                     1, (uint16_t)(height - 2), color);

        if (width > 1)
        {
            LCD_FillRect((uint16_t)(x + width - 1),
                         (uint16_t)(y + 1),
                         1, (uint16_t)(height - 2), color);
        }
    }
}


/* 查找字符对应的点阵数据 */
static const uint8_t *LCD_GetCharBitmap(char ch)
{
    uint32_t i;
    uint32_t count;

    count = sizeof(LCD_FontTable) / sizeof(LCD_FontTable[0]);

    /* 小写字母暂时转换为大写字母 */
    if (ch >= 'a' && ch <= 'z')
    {
        ch = (char)(ch - 'a' + 'A');
    }

    for (i = 0; i < count; i++)
    {
        if (LCD_FontTable[i].ch == ch)
        {
            return LCD_FontTable[i].col;
        }
    }

    /* 未收录的字符使用问号 */
    for (i = 0; i < count; i++)
    {
        if (LCD_FontTable[i].ch == '?')
        {
            return LCD_FontTable[i].col;
        }
    }

    return LCD_FontTable[0].col;
}

/* 绘制单个 ASCII 字符：5x7 点阵 */
void LCD_DrawChar(uint16_t x, uint16_t y,
                  char ch,
                  uint16_t color,
                  uint16_t bg_color)
{
    const uint8_t *bitmap;
    uint16_t row;
    uint16_t col;

    if (x >= LCD_WIDTH || y >= LCD_HEIGHT)
    {
        return;
    }

    bitmap = LCD_GetCharBitmap(ch);

    for (row = 0; row < LCD_FONT_HEIGHT; row++)
    {
        for (col = 0; col < LCD_FONT_WIDTH; col++)
        {
            if (x + col >= LCD_WIDTH ||
                y + row >= LCD_HEIGHT)
            {
                continue;
            }

            if (bitmap[col] & (1U << row))
            {
                LCD_DrawPoint(x + col, y + row, color);
            }
            else
            {
                LCD_DrawPoint(x + col, y + row, bg_color);
            }
        }
    }
}

/* 绘制字符串，字符间距 1 像素 */
void LCD_ShowString(uint16_t x, uint16_t y,
                    const char *str,
                    uint16_t color,
                    uint16_t bg_color)
{
    uint32_t cursor_x;

    if (str == NULL)
    {
        return;
    }

    cursor_x = x;

    while (*str != '\0')
    {
        if (cursor_x + LCD_FONT_WIDTH > LCD_WIDTH)
        {
            break;
        }

        LCD_DrawChar((uint16_t)cursor_x, y,
                     *str, color, bg_color);

        cursor_x += LCD_FONT_WIDTH + LCD_FONT_SPACING;
        str++;
    }
}

void LCD_ShowStringScale(uint16_t x, uint16_t y,
                         const char *str,
                         uint16_t color,
                         uint16_t bg_color,
                         uint8_t scale)
{
    uint32_t cursor_x;
    uint16_t row, col;
    uint8_t dx, dy;
    const uint8_t *bitmap;

    if (str == NULL || scale == 0)
    {
        return;
    }

    cursor_x = x;

    while (*str != '\0')
    {
        if (cursor_x >= LCD_WIDTH ||
            y >= LCD_HEIGHT)
        {
            break;
        }

        bitmap = LCD_GetCharBitmap(*str);

        for (col = 0; col < LCD_FONT_WIDTH; col++)
        {
            for (row = 0; row < LCD_FONT_HEIGHT; row++)
            {
                uint16_t pixel_color;

                pixel_color =
                    (bitmap[col] & (1U << row))
                    ? color : bg_color;

                for (dx = 0; dx < scale; dx++)
                {
                    for (dy = 0; dy < scale; dy++)
                    {
                        uint32_t px = cursor_x
                                    + col * scale + dx;
                        uint32_t py = (uint32_t)y
                                    + row * scale + dy;

                        if (px < LCD_WIDTH && py < LCD_HEIGHT)
                        {
                            LCD_DrawPoint((uint16_t)px,
                                          (uint16_t)py,
                                          pixel_color);
                        }
                    }
                }
            }
        }

        cursor_x += (LCD_FONT_WIDTH + LCD_FONT_SPACING)
                    * scale;

        str++;
    }
}