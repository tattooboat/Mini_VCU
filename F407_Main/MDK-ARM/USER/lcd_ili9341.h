#ifndef __LCD_ILI9341_H
#define __LCD_ILI9341_H

#include "stm32f4xx_hal.h"
#include "lcd_font.h"

#define LCD_WIDTH   320
#define LCD_HEIGHT  240

#define LCD_BLACK   0x0000
#define LCD_RED     0xF800
#define LCD_GREEN   0x07E0
#define LCD_BLUE    0x001F
#define LCD_WHITE   0xFFFF

void LCD_Init(void);
void LCD_FillColor(uint16_t color);

void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);

void LCD_FillRect(uint16_t x, uint16_t y,
                  uint16_t width, uint16_t height,
                  uint16_t color);

void LCD_DrawRect(uint16_t x, uint16_t y,
                  uint16_t width, uint16_t height,
                  uint16_t color);
/* ASCII 字符显示 */
void LCD_DrawChar(uint16_t x, uint16_t y,
                  char ch,
                  uint16_t color,
                  uint16_t bg_color);

void LCD_ShowString(uint16_t x, uint16_t y,
                    const char *str,
                    uint16_t color,
                    uint16_t bg_color);


void LCD_ShowStringScale(uint16_t x, uint16_t y,
                         const char *str,
                         uint16_t color,
                         uint16_t bg_color,
                         uint8_t scale);

                         /**
 * @brief  连续绘制 RGB565 像素区域
 * @param  x,y       左上角坐标
 * @param  width     区域宽度
 * @param  height    区域高度
 * @param  pixels    RGB565 像素数组，按行排列
 */
void LCD_DrawRGB565(uint16_t x, uint16_t y,
                    uint16_t width, uint16_t height,
                    const uint16_t *pixels);
#endif
