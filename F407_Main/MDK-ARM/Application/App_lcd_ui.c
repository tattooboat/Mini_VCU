
#include "App_lcd_ui.h"
#include "lcd_ili9341.h"

/* 自定义 RGB565 颜色 */
#define UI_BG       0x0841
#define UI_PANEL    0x10A2
#define UI_CYAN     0x07FF
#define UI_YELLOW   0xFFE0
#define UI_GRAY     0x8410
#define UI_BORDER   0x4208
#define MOTOR_CAN_ID       0x301U
#define MOTOR_FRAME_MARK   0xA5U
#define MOTOR_CMD_SET      0x01U
#define MOTOR_CMD_STOP     0x02U

static uint8_t LCD_UI_ParseMotorFrame(
    uint32_t can_id,
    uint8_t dlc,
    const uint8_t data[8],
    int16_t *target,
    uint8_t *motor_running)
{
    uint16_t raw;

    if ((can_id != MOTOR_CAN_ID) ||
        (dlc != 8U) ||
        (data == NULL) ||
        (target == NULL) ||
        (motor_running == NULL))
    {
        return 0U;
    }

    if (data[0] != MOTOR_FRAME_MARK)
    {
        return 0U;
    }

    raw = ((uint16_t)data[2] << 8) | data[3];
    *target = (int16_t)raw;

    if (data[1] == MOTOR_CMD_SET)
    {
        *motor_running = 1U;
    }
    else if (data[1] == MOTOR_CMD_STOP)
    {
        *target = 0;
        *motor_running = 0U;
    }
    else
    {
        return 0U;
    }

    return 1U;
}
void LCD_UI_ShowHomePage(void)
{
    /* 1. 整体背景 */
    LCD_FillColor(UI_BG);

    /* 2. 顶部标题栏 */
    LCD_FillRect(0, 0, 320, 30, UI_PANEL);
    LCD_ShowString(10, 7, "MINI VCU",
                   LCD_GREEN, UI_PANEL);

    LCD_ShowString(210, 10, "SYSTEM READY",
                   UI_CYAN, UI_PANEL);

    /* 3. 左侧：车辆速度区域 */
    LCD_FillRect(10, 40, 145, 100, UI_PANEL);
    LCD_DrawRect(10, 40, 145, 100, UI_CYAN);

    LCD_ShowString(20, 50, "VEHICLE SPEED",
                   UI_GRAY, UI_PANEL);

    LCD_ShowString(20, 75, "120 km/h",
                   LCD_WHITE, UI_PANEL);

    LCD_ShowString(20, 115, "DEMO VALUE",
                   UI_YELLOW, UI_PANEL);

    /* 4. 右侧：电机状态区域 */
    LCD_FillRect(165, 40, 145, 100, UI_PANEL);
    LCD_DrawRect(165, 40, 145, 100, LCD_GREEN);

    LCD_ShowString(175, 50, "MOTOR STATUS",
                   UI_GRAY, UI_PANEL);

    LCD_ShowString(175, 75, "STOP",
                   LCD_WHITE, UI_PANEL);

    LCD_ShowString(175, 105, "PWM: 0%",
                   UI_YELLOW, UI_PANEL);

    /* 5. 下方：CAN 通信区域 */
    LCD_FillRect(10, 150, 300, 65, UI_PANEL);
    LCD_DrawRect(10, 150, 300, 65, UI_BORDER);

    LCD_ShowString(20, 160, "COMMUNICATION",
                   UI_CYAN, UI_PANEL);

    LCD_ShowString(20, 180, "CAN BUS: READY",
                   LCD_GREEN, UI_PANEL);

    LCD_ShowString(175, 180, "MCU: F407",
                   LCD_WHITE, UI_PANEL);

    /* 6. 底部信息栏 */
    LCD_FillRect(0, 225, 320, 15, UI_BG);

    LCD_ShowString(10, 229, "MINI VCU DEMO",
                   UI_GRAY, UI_BG);
}

void LCD_UI_UpdateCAN(uint32_t can_id,
                      uint8_t dlc,
                      const uint8_t data[8],
                      uint32_t rx_count,
                      uint8_t can_online)
{
    char line[32];

    /* 刷新 CAN 状态 */
    LCD_FillRect(15, 176, 290, 14, UI_PANEL);

    if (can_online != 0U)
    {
        LCD_ShowString(20, 180, "CAN: ONLINE",
                       LCD_GREEN, UI_PANEL);
    }
    else
    {
        LCD_ShowString(20, 180, "CAN: OFFLINE",
                       UI_YELLOW, UI_PANEL);
    }

    /* 显示最近接收帧的 ID */
    snprintf(line, sizeof(line), "ID:%03lX DLC:%u",
             (unsigned long)can_id,
             (unsigned int)dlc);

    LCD_FillRect(15, 192, 290, 10, UI_PANEL);
    LCD_ShowString(20, 193, line, LCD_WHITE, UI_PANEL);

    /* 显示最近一帧的前四个数据字节 */
    snprintf(line, sizeof(line), "DATA:%02X %02X %02X %02X",
             data[0], data[1], data[2], data[3]);

    LCD_FillRect(15, 203, 290, 10, UI_PANEL);
    LCD_ShowString(20, 204, line, UI_CYAN, UI_PANEL);

    /* 显示接收帧计数 */
    snprintf(line, sizeof(line), "RX:%lu",
             (unsigned long)rx_count);

    LCD_FillRect(190, 160, 110, 12, UI_PANEL);
    LCD_ShowString(190, 160, line, UI_YELLOW, UI_PANEL);
}

void LCD_UI_UpdateMotor(uint32_t can_id,
                        uint8_t dlc,
                        const uint8_t data[8])
{
    char line[24];
    uint16_t raw;
    int16_t target;

    if ((can_id != 0x301U) ||
        (dlc != 8U) ||
        (data == NULL) ||
        (data[0] != 0xA5U))
    {
        return;
    }

    raw = ((uint16_t)data[2] << 8) | data[3];
    target = (int16_t)raw;

    LCD_FillRect(170, 70, 135, 65, UI_PANEL);

    if (data[1] == 0x01U)
    {
        LCD_ShowString(175, 75, "CMD: SET",
                       LCD_GREEN, UI_PANEL);

        snprintf(line, sizeof(line),
                 "TARGET: %d", (int)target);

        LCD_ShowString(175, 100, line,
                       LCD_WHITE, UI_PANEL);
    }
    else if (data[1] == 0x02U)
    {
        LCD_ShowString(175, 75, "CMD: STOP",
                       UI_YELLOW, UI_PANEL);

        LCD_ShowString(175, 100, "TARGET: 0",
                       LCD_WHITE, UI_PANEL);
    }
    else
    {
        LCD_ShowString(175, 75, "CMD: UNKNOWN",
                       UI_YELLOW, UI_PANEL);
    }
}

void LCD_UI_UpdateFault(uint32_t fault_flags)
{
    char fault_text[48];

    if (fault_flags == FAULT_NONE)
    {
        LCD_FillRect(10, 216, 300, 12, UI_BG);
        LCD_ShowString(10, 216,
                       "FAULT: NONE",
                       LCD_GREEN, UI_BG);
        return;
    }

    /* 优先显示 CAN 通信超时 */
    if ((fault_flags & FAULT_CAN_TIMEOUT) != 0U)
    {
        LCD_FillRect(10, 216, 300, 12, UI_BG);
        LCD_ShowString(10, 216,
                       "FAULT: CAN TIMEOUT",
                       LCD_RED, UI_BG);
        return;
    }

    if ((fault_flags & FAULT_CAN_BAD_FRAME) != 0U)
    {
        LCD_FillRect(10, 216, 300, 12, UI_BG);
        LCD_ShowString(10, 216,
                       "FAULT: BAD FRAME",
                       LCD_RED, UI_BG);
        return;
    }

    if ((fault_flags & FAULT_CAN_BAD_COMMAND) != 0U)
    {
        LCD_FillRect(10, 216, 300, 12, UI_BG);
        LCD_ShowString(10, 216,
                       "FAULT: BAD CMD",
                       LCD_RED, UI_BG);
        return;
    }

    if ((fault_flags & FAULT_CAN_BAD_SPEED) != 0U)
    {
        LCD_FillRect(10, 216, 300, 12, UI_BG);
        LCD_ShowString(10, 216,
                       "FAULT: BAD SPEED",
                       LCD_RED, UI_BG);
        return;
    }

    LCD_FillRect(10, 216, 300, 12, UI_BG);
    LCD_ShowString(10, 216,
                   "FAULT: UNKNOWN",
                   UI_YELLOW, UI_BG);
}
