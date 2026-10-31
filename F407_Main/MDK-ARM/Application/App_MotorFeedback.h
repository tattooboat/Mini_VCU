#ifndef APP_MOTOR_FEEDBACK_H
#define APP_MOTOR_FEEDBACK_H

#include "main.h"
#include <stdint.h>

#define MOTOR_FEEDBACK_CAN_ID    0x302U
#define MOTOR_FRAME_MARK         0xA5U

#define MOTOR_STATE_IDLE         0x00U
#define MOTOR_STATE_SET_ACCEPTED 0x01U
#define MOTOR_STATE_STOP_ACCEPTED 0x02U
#define MOTOR_STATE_FAULT        0x80U

#define MOTOR_FAULT_NONE         0x00U
#define MOTOR_FAULT_CAN_TIMEOUT  0x01U
#define MOTOR_FAULT_BAD_COMMAND  0x02U
#define MOTOR_FAULT_BAD_DATA     0x03U
#define MOTOR_FAULT_ESTOP        0x04U

HAL_StatusTypeDef App_MotorFeedback_Send(
    uint8_t state,
    int16_t target,
    uint8_t last_cmd,
    uint8_t seq,
    uint8_t fault_code
);

#endif