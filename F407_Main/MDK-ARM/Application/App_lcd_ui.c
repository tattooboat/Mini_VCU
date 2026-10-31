
#include "App_lcd_ui.h"
#include "lvgl.h"
#include <stdio.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "App_MotorControl.h"
#include "App_MotorFeedback.h"

/* printf 重定向到 USART1（串口助手波特率 115200） */
extern UART_HandleTypeDef huart1;

int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 100);
    return ch;
}

#define UI_BG       0x0000
#define UI_PANEL    0x18A2
#define UI_CYAN     0x7FFF
#define UI_YELLOW   0xFFE0
#define UI_GRAY     0xA514
#define UI_GREEN    0x07E0
#define UI_RED      0xF800
#define UI_WHITE    0xFFFF

/* 与 freertos.c 中 SPEED_PID_PWM_MAX 对应，用于换算 PWM 占空比 % */
#define UI_PWM_MAX  1000

#define MOTOR_CAN_ID     0x301U
#define MOTOR_FRAME_MARK 0xA5U
#define MOTOR_CMD_SET    0x01U
#define MOTOR_CMD_STOP   0x02U

static lv_obj_t *label_title;
static lv_obj_t *label_speed;
static lv_obj_t *label_target;
static lv_obj_t *label_motor;
static lv_obj_t *label_pwm;
static lv_obj_t *label_can;
static lv_obj_t *label_can_frame;
static lv_obj_t *label_can_data;
static lv_obj_t *label_rx;
static lv_obj_t *label_fault;
static lv_obj_t *label_mcu;

/* ---- 控制页本地控速参数 ---- */
/* 与 freertos.c 中 MOTOR_MAX_RPM 对应，用于 协议码 <-> RPM 换算 */
#define UI_MOTOR_MAX_RPM  169.0f
/* 安全测试范围：目标转速限制在 ±150 RPM */
#define UI_SPEED_LIMIT_RPM   150
/* 每次点击加减的步进 */
#define UI_SPEED_STEP_RPM    10

/* 控制页控件句柄 */
static lv_obj_t *ctrl_label_speed;
static lv_obj_t *ctrl_label_state;
static lv_obj_t *ctrl_label_actual;
static lv_obj_t *ctrl_label_tx;
/* 控制页当前目标转速（RPM，有符号）与运行状态 */
static int16_t ctrl_target_rpm = 0;
static uint8_t ctrl_running = 0U;
static uint8_t ctrl_tx_seq = 0U;

static lv_obj_t *make_label(lv_obj_t *parent,
                            const char *text,
                            uint32_t color,
                            int x,
                            int y)
{
    lv_obj_t *obj = lv_label_create(parent);

    lv_label_set_text(obj, text);
    lv_obj_set_style_text_color(
        obj, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(
        obj, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(obj, x, y);

    return obj;
}

/* 页面切换相关 */
static void UI_ResetLabels(void)
{
    label_title = NULL;
    label_speed = NULL;
    label_target = NULL;
    label_motor = NULL;
    label_pwm = NULL;
    label_can = NULL;
    label_can_frame = NULL;
    label_can_data = NULL;
    label_rx = NULL;
    label_fault = NULL;
    label_mcu = NULL;

    ctrl_label_speed = NULL;
    ctrl_label_state = NULL;
    ctrl_label_actual = NULL;
    ctrl_label_tx = NULL;
}

/* 页面切换按钮 */
static lv_obj_t *UI_CreateButtonEx(
    lv_obj_t *parent,
    const char *text,
    int x,
    int y,
    int width,
    int height,
    uint32_t bg_color,
    lv_event_cb_t callback)
{
    lv_obj_t *btn = lv_button_create(parent);

    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, width, height);

    lv_obj_set_style_bg_color(
        btn, lv_color_hex(bg_color), 0);

    lv_obj_set_style_radius(btn, 6, 0);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(
        label, lv_color_hex(UI_BG), 0);
    lv_obj_center(label);

    lv_obj_add_event_cb(
        btn, callback, LV_EVENT_CLICKED, NULL);

    return btn;
}

static lv_obj_t *UI_CreateButton(
    lv_obj_t *parent,
    const char *text,
    int x,
    int y,
    int width,
    int height,
    lv_event_cb_t callback)
{
    return UI_CreateButtonEx(
        parent,
        text,
        x, y,
        width, height,
        UI_CYAN,
        callback);
}

/* 点击控制页 */
static void UI_GoControl_Event(lv_event_t *e)
{
    (void)e;
    LCD_UI_ShowControlPage();
}

/* 点击返回主页 */
static void UI_GoHome_Event(lv_event_t *e)
{
    (void)e;
    LCD_UI_ShowHomePage();
}

static void Touch_Test_Event(lv_event_t *e)
{
    (void)e;
    printf("Touch button clicked!\r\n");
}

void LCD_UI_ShowHomePage(void)
{
    lv_obj_t *scr = lv_screen_active();

    /* 清空旧页面标签指针 */
    UI_ResetLabels();

    /* 清理旧屏幕对象 */
    lv_obj_clean(scr);

    lv_obj_set_style_bg_color(
        scr, lv_color_hex(UI_BG), 0);

    /* 标题栏 */
    lv_obj_t *header = lv_obj_create(scr);
    lv_obj_set_pos(header, 0, 0);
    lv_obj_set_size(header, 320, 32);
    lv_obj_set_style_bg_color(
        header, lv_color_hex(UI_PANEL), 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);
    lv_obj_set_scrollbar_mode(header, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);

    label_title = make_label(
        header, "MINI VCU", UI_WHITE, 10, 8);
    lv_obj_set_style_text_font(
        label_title, &lv_font_montserrat_14, 0);

     /* 主页右上角：进入控制页 */
    UI_CreateButton(
        header,
        "CONTROL",
        230, 3,
        82, 26,
        UI_GoControl_Event
    );

    /* 左侧速度卡片：显示真实实际转速 */
    lv_obj_t *speed_card = lv_obj_create(scr);
    lv_obj_set_pos(speed_card, 8, 34);
    lv_obj_set_size(speed_card, 148, 92);
    lv_obj_set_style_bg_color(
        speed_card, lv_color_hex(UI_PANEL), 0);
    lv_obj_set_style_border_color(
        speed_card, lv_color_hex(UI_CYAN), 0);
    lv_obj_set_style_pad_all(speed_card, 0, 0);
    lv_obj_set_scrollbar_mode(speed_card, LV_SCROLLBAR_MODE_OFF);

    make_label(speed_card, "MOTOR SPEED",
               UI_GRAY, 8, 6);

    label_speed = make_label(
        speed_card, "0 RPM", UI_WHITE, 8, 30);

    label_target = make_label(
        speed_card, "TGT 0 RPM", UI_YELLOW, 8, 58);

    /* 右侧电机状态卡片：真实运行状态 + PWM */
    lv_obj_t *motor_card = lv_obj_create(scr);
    lv_obj_set_pos(motor_card, 164, 34);
    lv_obj_set_size(motor_card, 148, 92);
    lv_obj_set_style_bg_color(
        motor_card, lv_color_hex(UI_PANEL), 0);
    lv_obj_set_style_border_color(
        motor_card, lv_color_hex(UI_WHITE), 0);
    lv_obj_set_style_pad_all(motor_card, 0, 0);
    lv_obj_set_scrollbar_mode(motor_card, LV_SCROLLBAR_MODE_OFF);

    make_label(motor_card, "MOTOR STATUS",
               UI_GRAY, 8, 6);

    label_motor = make_label(
        motor_card, "STOPPED", UI_GRAY, 8, 30);

    label_pwm = make_label(
        motor_card, "PWM 0%", UI_YELLOW, 8, 58);

    /* CAN 通信卡片 */
    lv_obj_t *can_card = lv_obj_create(scr);
    lv_obj_set_pos(can_card, 8, 132);
    lv_obj_set_size(can_card, 304, 78);
    lv_obj_set_style_bg_color(
        can_card, lv_color_hex(UI_PANEL), 0);
    lv_obj_set_style_border_color(
        can_card, lv_color_hex(UI_GRAY), 0);
    lv_obj_set_style_pad_all(can_card, 0, 0);
    lv_obj_set_scrollbar_mode(can_card, LV_SCROLLBAR_MODE_OFF);

    make_label(can_card, "COMMUNICATION",
               UI_CYAN, 8, 4);

    label_rx = make_label(
        can_card, "RX: 0", UI_YELLOW, 222, 4);

    label_can = make_label(
        can_card, "CAN: OFFLINE", UI_YELLOW, 8, 24);

    label_can_frame = make_label(
        can_card, "ID: --- DLC: 0",
        UI_WHITE, 8, 46);

    label_can_data = make_label(
        can_card, "DATA: -- -- -- --",
        UI_CYAN, 170, 46);

    /* 故障与底部信息 */
    label_fault = make_label(
        scr, "FAULT: NONE", UI_WHITE, 10, 217);

    label_mcu = make_label(
        scr, "MCU: F407", UI_GRAY, 235, 217);

    (void)label_title;

}

void LCD_UI_UpdateCAN(uint32_t can_id,
                      uint8_t dlc,
                      const uint8_t data[8],
                      uint32_t rx_count,
                      uint8_t can_online)
{
    char line[48];

    if (label_can == NULL ||
        label_can_frame == NULL ||
        label_can_data == NULL ||
        label_rx == NULL ||
        data == NULL)
    {
        return;
    }

    if (can_online != 0U)
    {
        lv_label_set_text(label_can, "CAN: ONLINE");
        lv_obj_set_style_text_color(
            label_can, lv_color_hex(UI_WHITE), 0);
    }
    else
    {
        lv_label_set_text(label_can, "CAN: OFFLINE");
        lv_obj_set_style_text_color(
            label_can, lv_color_hex(UI_YELLOW), 0);
    }

    lv_label_set_text_fmt(
        label_can_frame,
        "ID:%03lX DLC:%u",
        (unsigned long)can_id,
        (unsigned int)dlc);

    lv_label_set_text_fmt(
        label_can_data,
        "DATA:%02X %02X %02X %02X",
        data[0], data[1], data[2], data[3]);

    lv_label_set_text_fmt(
        label_rx, "RX:%lu",
        (unsigned long)rx_count);
}

void LCD_UI_UpdateMotor(uint32_t can_id,
                        uint8_t dlc,
                        const uint8_t data[8])
{
    int16_t target;

    if (label_motor == NULL ||
        label_pwm == NULL ||
        data == NULL)
    {
        return;
    }

    if (can_id != MOTOR_CAN_ID ||
        dlc != 8U ||
        data[0] != MOTOR_FRAME_MARK)
    {
        return;
    }

    target = (int16_t)(
        ((uint16_t)data[2] << 8) | data[3]);

    if (data[1] == MOTOR_CMD_SET)
    {
        lv_label_set_text_fmt(
            label_motor, "CMD: SET %d", (int)target);

        lv_label_set_text(
            label_pwm, "PWM: command");
    }
    else if (data[1] == MOTOR_CMD_STOP)
    {
        lv_label_set_text(label_motor, "CMD: STOP");
        lv_label_set_text(label_pwm, "PWM: 0%");
    }
    else
    {
        lv_label_set_text(label_motor, "CMD: UNKNOWN");
    }
}

void LCD_UI_UpdateSpeed(float target_rpm,
                        float actual_rpm,
                        int16_t pwm)
{
    int pwm_percent;

    if (label_speed == NULL)
    {
        return;
    }

    /* 实际转速取绝对值显示，转速方向由目标值体现 */
    if (actual_rpm < 0.0f)
    {
        actual_rpm = -actual_rpm;
    }

    /* 实际转速为整数，用 %d 展示；规避内置 printf 对浮点精度的兼容问题 */
    lv_label_set_text_fmt(
        label_speed,
        "%d RPM",
        (int)(actual_rpm + 0.5f));

    if (label_target != NULL)
    {
        float tgt = target_rpm;

        if (tgt < 0.0f)
        {
            lv_label_set_text_fmt(
                label_target,
                "TGT %d RPM",
                (int)(tgt - 0.5f));
        }
        else
        {
            lv_label_set_text_fmt(
                label_target,
                "TGT %d RPM",
                (int)(tgt + 0.5f));
        }
    }

    pwm_percent = (int)(((int32_t)pwm * 100) / UI_PWM_MAX);

    if (pwm_percent > 100)
    {
        pwm_percent = 100;
    }
    else if (pwm_percent < -100)
    {
        pwm_percent = -100;
    }

    if (label_pwm != NULL)
    {
        lv_label_set_text_fmt(
            label_pwm,
            "PWM %d%%",
            pwm_percent);
    }

    /* 主页电机状态：按真实驱动状态联动（CAN 或本地控制均适用） */
    if (label_motor != NULL)
    {
        if (App_MotorControl_IsEMLatched())
        {
            lv_label_set_text(label_motor, "E-STOP");
            lv_obj_set_style_text_color(
                label_motor, lv_color_hex(UI_RED), 0);
        }
        else if (pwm != 0)
        {
            lv_label_set_text(label_motor, "RUNNING");
            lv_obj_set_style_text_color(
                label_motor, lv_color_hex(UI_GREEN), 0);
        }
        else
        {
            lv_label_set_text(label_motor, "STOPPED");
            lv_obj_set_style_text_color(
                label_motor, lv_color_hex(UI_GRAY), 0);
        }
    }
}

void LCD_UI_UpdateFault(uint32_t fault_flags)
{
    const char *text;
    uint32_t color;

    if (label_fault == NULL)
    {
        return;
    }

    if (fault_flags == FAULT_NONE)
    {
        text = "FAULT: NONE";
        color = UI_WHITE;
    }
    else if ((fault_flags & FAULT_EMERGENCY_STOP) != 0U)
    {
        text = "FAULT: E-STOP";
        color = UI_RED;
    }
    else if ((fault_flags & FAULT_CAN_TIMEOUT) != 0U)
    {
        text = "FAULT: CAN TIMEOUT";
        color = UI_RED;
    }
    else if ((fault_flags & FAULT_CAN_BAD_FRAME) != 0U)
    {
        text = "FAULT: BAD FRAME";
        color = UI_RED;
    }
    else if ((fault_flags & FAULT_CAN_BAD_COMMAND) != 0U)
    {
        text = "FAULT: BAD CMD";
        color = UI_RED;
    }
    else if ((fault_flags & FAULT_CAN_BAD_SPEED) != 0U)
    {
        text = "FAULT: BAD SPEED";
        color = UI_RED;
    }
    else
    {
        text = "FAULT: UNKNOWN";
        color = UI_YELLOW;
    }

    lv_label_set_text(label_fault, text);
    lv_obj_set_style_text_color(
        label_fault, lv_color_hex(color), 0);
}

/* =========================================
 * 控制页面
 * 4-4-2：本地直控（方案1）——调速 / 启停 / 反馈帧发送结果
 * ========================================= */

/* 目标转速(RPM) -> 协议码(-1000~1000) */
static int16_t ctrl_rpm_to_raw(int16_t rpm)
{
    return (int16_t)((float)rpm / UI_MOTOR_MAX_RPM * 1000.0f);
}

static void ctrl_update_speed_label(void)
{
    if (ctrl_label_speed == NULL)
    {
        return;
    }

    lv_label_set_text_fmt(
        ctrl_label_speed,
        "TGT %d RPM",
        (int)ctrl_target_rpm);
}

static void ctrl_update_state_label(void)
{
    if (ctrl_label_state == NULL)
    {
        return;
    }

    if (App_MotorControl_IsEMLatched())
    {
        lv_label_set_text(ctrl_label_state, "MOTOR: E-STOP");
        lv_obj_set_style_text_color(
            ctrl_label_state, lv_color_hex(UI_RED), 0);
    }
    else if (ctrl_running != 0U)
    {
        lv_label_set_text(ctrl_label_state, "MOTOR: RUNNING");
        lv_obj_set_style_text_color(
            ctrl_label_state, lv_color_hex(UI_GREEN), 0);
    }
    else
    {
        lv_label_set_text(ctrl_label_state, "MOTOR: STOPPED");
        lv_obj_set_style_text_color(
            ctrl_label_state, lv_color_hex(UI_GRAY), 0);
    }
}

static void ctrl_update_tx_label(HAL_StatusTypeDef status)
{
    uint32_t color = UI_RED;

    if (ctrl_label_tx == NULL)
    {
        return;
    }

    if (status == HAL_OK)
    {
        lv_label_set_text(ctrl_label_tx, "TX: OK");
        color = UI_GREEN;
    }
    else if (status == HAL_BUSY)
    {
        lv_label_set_text(ctrl_label_tx, "TX: HOLD");
        color = UI_YELLOW;
    }
    else
    {
        lv_label_set_text(ctrl_label_tx, "TX: FAIL");
        color = UI_RED;
    }

    lv_obj_set_style_text_color(
        ctrl_label_tx, lv_color_hex(color), 0);
}

/* 减小目标转速 */
static void Ctl_SpeedDown_Event(lv_event_t *e)
{
    (void)e;

    ctrl_target_rpm -= UI_SPEED_STEP_RPM;

    if (ctrl_target_rpm < -UI_SPEED_LIMIT_RPM)
    {
        ctrl_target_rpm = -UI_SPEED_LIMIT_RPM;
    }

    ctrl_update_speed_label();

    /* 运行中实时调速 */
    if (ctrl_running != 0U)
    {
        App_MotorControl_SetSpeed(ctrl_rpm_to_raw(ctrl_target_rpm));
    }
}

/* 增大目标转速 */
static void Ctl_SpeedUp_Event(lv_event_t *e)
{
    (void)e;

    ctrl_target_rpm += UI_SPEED_STEP_RPM;

    if (ctrl_target_rpm > UI_SPEED_LIMIT_RPM)
    {
        ctrl_target_rpm = UI_SPEED_LIMIT_RPM;
    }

    ctrl_update_speed_label();

    /* 运行中实时调速 */
    if (ctrl_running != 0U)
    {
        App_MotorControl_SetSpeed(ctrl_rpm_to_raw(ctrl_target_rpm));
    }
}

/* 启动：本地使能并发送反馈帧 */
static void Ctl_Start_Event(lv_event_t *e)
{
    HAL_StatusTypeDef status;
    int16_t raw;

    (void)e;

    raw = ctrl_rpm_to_raw(ctrl_target_rpm);

    App_MotorControl_SetSpeed(raw);
    ctrl_running = 1U;

    /* START 亦作为重新启动入口：复位急停/超时故障显示 */
    App_Fault_Clear(FAULT_EMERGENCY_STOP);
    App_Fault_Clear(FAULT_CAN_TIMEOUT);

    status = App_MotorFeedback_Send(
        MOTOR_STATE_SET_ACCEPTED,
        raw,
        MOTOR_CMD_SET,
        ctrl_tx_seq++,
        MOTOR_FAULT_NONE);

    ctrl_update_speed_label();
    ctrl_update_state_label();
    ctrl_update_tx_label(status);
}

/* 停止：本地停止并发送反馈帧 */
static void Ctl_Stop_Event(lv_event_t *e)
{
    HAL_StatusTypeDef status;

    (void)e;

    App_MotorControl_Stop();
    ctrl_running = 0U;

    status = App_MotorFeedback_Send(
        MOTOR_STATE_STOP_ACCEPTED,
        0,
        MOTOR_CMD_STOP,
        ctrl_tx_seq++,
        MOTOR_FAULT_NONE);

    ctrl_update_speed_label();
    ctrl_update_state_label();
    ctrl_update_tx_label(status);
}

/* 急停：最高优先级停机并锁存，忽略一切目标直到复位 */
static void Ctl_EMStop_Event(lv_event_t *e)
{
    HAL_StatusTypeDef status;

    (void)e;

    App_MotorControl_EMStop();
    App_Fault_Set(FAULT_EMERGENCY_STOP);
    ctrl_running = 0U;

    status = App_MotorFeedback_Send(
        MOTOR_STATE_FAULT,
        0,
        MOTOR_CMD_STOP,
        ctrl_tx_seq++,
        MOTOR_FAULT_ESTOP);

    ctrl_update_speed_label();
    ctrl_update_state_label();
    ctrl_update_tx_label(status);
}

/* 重新启动/复位：先复位急停锁存，否则释放普通停机锁存交还 CAN */
static void Ctl_Reset_Event(lv_event_t *e)
{
    (void)e;

    if (App_MotorControl_IsEMLatched())
    {
        App_MotorControl_ReleaseEMStop();
        App_Fault_Clear(FAULT_EMERGENCY_STOP);
    }
    else
    {
        App_MotorControl_ReleaseStopLatch();
    }

    App_Fault_Clear(FAULT_CAN_TIMEOUT);
    ctrl_running = 0U;

    ctrl_update_speed_label();
    ctrl_update_state_label();

    if (ctrl_label_tx != NULL)
    {
        lv_label_set_text(ctrl_label_tx, "RESET: OK");
        lv_obj_set_style_text_color(
            ctrl_label_tx, lv_color_hex(UI_GREEN), 0);
    }
}

void LCD_UI_UpdateControlStatus(float actual_rpm, int16_t pwm)
{
    int pwm_percent;

    if (ctrl_label_actual == NULL)
    {
        return;
    }

    if (actual_rpm < 0.0f)
    {
        actual_rpm = -actual_rpm;
    }

    pwm_percent = (int)(((int32_t)pwm * 100) / UI_PWM_MAX);

    if (pwm_percent > 100)
    {
        pwm_percent = 100;
    }
    else if (pwm_percent < -100)
    {
        pwm_percent = -100;
    }

    lv_label_set_text_fmt(
        ctrl_label_actual,
        "ACT %d RPM    PWM %d%%",
        (int)actual_rpm,
        pwm_percent);
}
void LCD_UI_ShowControlPage(void)
{
    lv_obj_t *scr = lv_screen_active();

    /* 清理旧页面标签指针 */
    UI_ResetLabels();

    /* 清理旧页面 */
    lv_obj_clean(scr);

    lv_obj_set_style_bg_color(
        scr, lv_color_hex(UI_BG), 0);

    /* 整页固定，禁止滚动/动画 */
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);

    /* 顶部标题栏 */
    lv_obj_t *header = lv_obj_create(scr);

    lv_obj_set_pos(header, 0, 0);
    lv_obj_set_size(header, 320, 36);

    lv_obj_set_style_bg_color(
        header, lv_color_hex(UI_PANEL), 0);

    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);
    lv_obj_set_scrollbar_mode(header, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *title = make_label(
        header,
        "MOTOR CONTROL",
        UI_WHITE,
        10,
        10
    );

    lv_obj_set_style_text_font(
        title, &lv_font_montserrat_14, 0);

    /* 返回主页按钮 */
    UI_CreateButton(
        header,
        "HOME",
        246, 4,
        66, 28,
        UI_GoHome_Event
    );

    /* 状态卡片：目标、状态、实际转速/PWM、反馈结果 */
    lv_obj_t *status_card = lv_obj_create(scr);

    lv_obj_set_pos(status_card, 10, 38);
    lv_obj_set_size(status_card, 300, 70);

    lv_obj_set_style_bg_color(
        status_card, lv_color_hex(UI_PANEL), 0);

    lv_obj_set_style_border_color(
        status_card, lv_color_hex(UI_WHITE), 0);

    lv_obj_set_style_pad_all(status_card, 0, 0);
    lv_obj_set_scrollbar_mode(status_card, LV_SCROLLBAR_MODE_OFF);

    ctrl_label_speed = make_label(
        status_card, "TGT 0 RPM", UI_WHITE, 8, 6);

    ctrl_label_state = make_label(
        status_card, "STOPPED", UI_GRAY, 150, 8);

    ctrl_label_actual = make_label(
        status_card, "ACT 0 RPM  PWM 0%",
        UI_YELLOW, 8, 30);

    ctrl_label_tx = make_label(
        status_card, "TX: --", UI_CYAN, 8, 52);

    /* 控制按钮区：三行大按钮（34px 高），两列布局，触摸友好、无滚动 */
    lv_obj_t *control_card = lv_obj_create(scr);

    lv_obj_set_pos(control_card, 10, 112);
    lv_obj_set_size(control_card, 300, 122);

    lv_obj_set_style_bg_color(
        control_card, lv_color_hex(UI_PANEL), 0);

    lv_obj_set_style_border_color(
        control_card, lv_color_hex(UI_CYAN), 0);

    lv_obj_set_style_pad_all(control_card, 0, 0);
    lv_obj_set_scrollbar_mode(control_card, LV_SCROLLBAR_MODE_OFF);

    /* 第 1 行：启停 */
    UI_CreateButtonEx(
        control_card, "START", 8, 6, 140, 34, UI_GREEN,
        Ctl_Start_Event
    );

    UI_CreateButtonEx(
        control_card, "STOP", 152, 6, 140, 34, UI_RED,
        Ctl_Stop_Event
    );

    /* 第 2 行：调速 */
    UI_CreateButtonEx(
        control_card, "-", 8, 44, 140, 34, UI_CYAN,
        Ctl_SpeedDown_Event
    );

    UI_CreateButtonEx(
        control_card, "+", 152, 44, 140, 34, UI_CYAN,
        Ctl_SpeedUp_Event
    );

    /* 第 3 行：急停 + 复位/交还 CAN */
    UI_CreateButtonEx(
        control_card, "E-STOP", 8, 82, 196, 34, UI_RED,
        Ctl_EMStop_Event
    );

    UI_CreateButtonEx(
        control_card, "RESET", 208, 82, 84, 34, UI_CYAN,
        Ctl_Reset_Event
    );

    /* 根据当前本地控制状态初始化 */
    ctrl_running =
        (App_MotorControl_IsActive() != 0U) ? 1U : 0U;

    ctrl_update_speed_label();
    ctrl_update_state_label();
}