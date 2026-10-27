#include "App_MotorFeedbackRx.h"
#include <string.h>

static volatile uint8_t  s_valid = 0U;
static volatile uint8_t  s_state = MOTOR_STATE_IDLE;
static volatile int16_t  s_target_speed = 0;
static volatile uint8_t  s_last_cmd = 0U;
static volatile uint8_t  s_seq = 0U;
static volatile uint8_t  s_fault_code = MOTOR_FAULT_NONE;
static volatile uint32_t s_rx_count = 0U;
static volatile uint32_t s_last_rx_tick = 0U;

void App_MotorFeedbackRx_Init(void)
{
    s_valid = 0U;
    s_state = MOTOR_STATE_IDLE;
    s_target_speed = 0;
    s_last_cmd = 0U;
    s_seq = 0U;
    s_fault_code = MOTOR_FAULT_NONE;
    s_rx_count = 0U;
    s_last_rx_tick = 0U;
}

void App_MotorFeedbackRx_Process(uint32_t std_id,
                                const uint8_t *data,
                                uint8_t len)
{
    int16_t target;

    if (std_id != MOTOR_FEEDBACK_CAN_ID)
    {
        return;
    }

    if ((data == NULL) || (len != 8U))
    {
        return;
    }

    if (data[0] != MOTOR_FEEDBACK_FRAME_MARK)
    {
        return;
    }

    target = (int16_t)(((uint16_t)data[2] << 8) |
                       (uint16_t)data[3]);

    s_state = data[1];
    s_target_speed = target;
    s_last_cmd = data[4];
    s_seq = data[5];
    s_fault_code = data[6];
    s_last_rx_tick = HAL_GetTick();
    s_rx_count++;
    s_valid = 1U;
}

void App_MotorFeedbackRx_Get(MotorFeedbackRx_t *feedback)
{
    uint32_t primask;

    if (feedback == NULL)
    {
        return;
    }

    /*
     * 防止读取多个字段时被任务或中断更新到一半。
     * 该临界区只用于快速复制数据，不应放入耗时操作。
     */
    primask = __get_PRIMASK();
    __disable_irq();

    feedback->valid = s_valid;
    feedback->state = s_state;
    feedback->target_speed = s_target_speed;
    feedback->last_cmd = s_last_cmd;
    feedback->seq = s_seq;
    feedback->fault_code = s_fault_code;
    feedback->rx_count = s_rx_count;
    feedback->last_rx_tick = s_last_rx_tick;

    if (primask == 0U)
    {
        __enable_irq();
    }
}
