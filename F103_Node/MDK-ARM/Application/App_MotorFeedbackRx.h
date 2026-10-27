#ifndef APP_MOTOR_FEEDBACK_RX_H
#define APP_MOTOR_FEEDBACK_RX_H

#include "main.h"
#include <stdint.h>

#define MOTOR_FEEDBACK_CAN_ID       0x302U
#define MOTOR_FEEDBACK_FRAME_MARK   0xA5U

#define MOTOR_STATE_IDLE            0x00U
#define MOTOR_STATE_SET_ACCEPTED    0x01U
#define MOTOR_STATE_STOP_ACCEPTED   0x02U
#define MOTOR_STATE_FAULT           0x80U

#define MOTOR_FAULT_NONE            0x00U
#define MOTOR_FAULT_CAN_TIMEOUT     0x01U
#define MOTOR_FAULT_BAD_COMMAND     0x02U
#define MOTOR_FAULT_BAD_DATA        0x03U

typedef struct
{
    uint8_t valid;
    uint8_t state;
    int16_t target_speed;
    uint8_t last_cmd;
    uint8_t seq;
    uint8_t fault_code;
    uint32_t rx_count;
    uint32_t last_rx_tick;
} MotorFeedbackRx_t;

void App_MotorFeedbackRx_Init(void);

/* 在任务上下文调用，不要直接在 CAN 中断里调用 */
void App_MotorFeedbackRx_Process(uint32_t std_id,
                                const uint8_t *data,
                                uint8_t len);

void App_MotorFeedbackRx_Get(MotorFeedbackRx_t *feedback);

#endif
