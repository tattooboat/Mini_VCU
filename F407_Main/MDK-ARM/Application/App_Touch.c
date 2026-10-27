#include "App_Touch.h"

#define LCD_WIDTH       320
#define LCD_HEIGHT      240

#define RAW_X_TOP       3500
#define RAW_X_BOTTOM     650
#define RAW_Y_LEFT      3600
#define RAW_Y_RIGHT      600

static uint16_t Clamp(int32_t value, uint16_t max)
{
    if (value < 0) return 0;
    if (value > max) return max;
    return (uint16_t)value;
}

void App_Touch_Map(uint16_t raw_x,
                   uint16_t raw_y,
                   uint16_t *screen_x,
                   uint16_t *screen_y)
{
    int32_t x;
    int32_t y;

    if (screen_x == 0 || screen_y == 0)
        return;

    /* 原始 Y 对应屏幕左右方向 */
    x = ((int32_t)RAW_Y_LEFT - raw_y)
        * (LCD_WIDTH - 1)
        / (RAW_Y_LEFT - RAW_Y_RIGHT);

    /* 原始 X 对应屏幕上下方向 */
    y = ((int32_t)RAW_X_TOP - raw_x)
        * (LCD_HEIGHT - 1)
        / (RAW_X_TOP - RAW_X_BOTTOM);

    *screen_x = Clamp(x, LCD_WIDTH - 1);
    *screen_y = Clamp(y, LCD_HEIGHT - 1);
}
