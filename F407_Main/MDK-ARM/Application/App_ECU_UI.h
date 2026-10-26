#ifndef APP_ECU_UI_H
#define APP_ECU_UI_H

#include <stdint.h>

typedef struct
{
    float target_rpm;
    float actual_rpm;

    int16_t pwm_command;

    uint8_t can_online;
    uint8_t motor_running;

    uint32_t fault_flags;
} ECU_UI_Status_t;

#endif
