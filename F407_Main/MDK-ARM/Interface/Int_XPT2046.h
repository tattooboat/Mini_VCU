#ifndef INT_XPT2046_H
#define INT_XPT2046_H

#include "main.h"
#include <stdint.h>



void Int_XPT2046_Init(void);

/* 读取一次原始 X/Y 坐标，返回 1 表示读取完成 */
uint8_t Int_XPT2046_ReadRaw(uint16_t *x, uint16_t *y);
uint8_t Int_XPT2046_IsPressed(void);

extern volatile uint16_t g_touch_raw_x;
extern volatile uint16_t g_touch_raw_y;
#endif