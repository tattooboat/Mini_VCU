#ifndef __LCD_ILI9341_H
#define __LCD_ILI9341_H

#include "stm32f4xx_hal.h"

#define LCD_WIDTH   320
#define LCD_HEIGHT  240

#define LCD_BLACK   0x0000
#define LCD_RED     0xF800
#define LCD_GREEN   0x07E0
#define LCD_BLUE    0x001F
#define LCD_WHITE   0xFFFF

void LCD_Init(void);
void LCD_FillColor(uint16_t color);

#endif
