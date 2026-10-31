#include "App_Touch.h"

#include "Int_XPT2046.h"
#include "lvgl.h"

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

/* LVGL 触摸输入设备 */
static lv_indev_t *g_touch_indev = NULL;

/* 保存上一次有效坐标 */
static lv_point_t g_touch_point = {0, 0};

/*
 * LVGL 触摸读取回调
 * LVGL 会周期性调用这个函数
 */
static void App_Touch_Read(lv_indev_t *indev,
                           lv_indev_data_t *data)
{
    uint16_t raw_x = 0U;
    uint16_t raw_y = 0U;
    uint16_t screen_x = 0U;
    uint16_t screen_y = 0U;

    (void)indev;

    if (Int_XPT2046_IsPressed())
    {
        if (Int_XPT2046_ReadRaw(&raw_x, &raw_y))
        {
            App_Touch_Map(
                raw_x,
                raw_y,
                &screen_x,
                &screen_y
            );

            g_touch_point.x = (lv_coord_t)screen_x;
            g_touch_point.y = (lv_coord_t)screen_y;
        }

        data->state = LV_INDEV_STATE_PRESSED;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }

    /* 松开时也保留最后一次坐标 */
    data->point.x = g_touch_point.x;
    data->point.y = g_touch_point.y;
}

/* 初始化 LVGL 触摸输入设备 */
void App_Touch_LVGL_Init(void)
{
    if (g_touch_indev != NULL)
    {
        return;
    }

    Int_XPT2046_Init();

    g_touch_indev = lv_indev_create();

    lv_indev_set_type(
        g_touch_indev,
        LV_INDEV_TYPE_POINTER
    );

    lv_indev_set_read_cb(
        g_touch_indev,
        App_Touch_Read
    );
}
