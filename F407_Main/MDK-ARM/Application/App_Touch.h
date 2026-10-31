#ifndef APP_TOUCH_H
#define APP_TOUCH_H

#include <stdint.h>

void App_Touch_Map(
    uint16_t raw_x,
    uint16_t raw_y,
    uint16_t *screen_x,
    uint16_t *screen_y
);

/* 注册 XPT2046 为 LVGL 触摸输入设备 */
void App_Touch_LVGL_Init(void);

#endif