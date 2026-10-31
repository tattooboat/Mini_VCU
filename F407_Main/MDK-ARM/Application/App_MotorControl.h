#ifndef APP_MOTOR_CONTROL_H
#define APP_MOTOR_CONTROL_H

#include <stdint.h>
#include "Int_motor.h"

void App_MotorControl_Init(void);
void App_MotorControl_SetSpeed(int16_t speed);
void App_MotorControl_Stop(void);
/* 急停：立即停机并锁存，忽略一切目标，直到显式复位 */
void App_MotorControl_EMStop(void);
/* 急停/异常停机锁存是否生效（最高优先级） */
uint8_t App_MotorControl_IsEMLatched(void);
/* 重新启动入口：释放急停锁存并回归 CAN */
void App_MotorControl_ReleaseEMStop(void);

/* 本地(LCD)控制是否处于生效状态 */
uint8_t App_MotorControl_IsActive(void);
/* 本地控制当前目标速度（协议码 -1000~1000） */
int16_t App_MotorControl_GetTargetSpeed(void);
/* 强制停机锁存是否生效（STOP 后为真，直到再次 START 清除） */
uint8_t App_MotorControl_IsStopLatched(void);
/* 释放停机锁存并回归 CAN：清 latch、退出本地控制，交还 F103 */
void App_MotorControl_ReleaseStopLatch(void);

#endif