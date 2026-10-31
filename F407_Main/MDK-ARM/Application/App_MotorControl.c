#include "App_MotorControl.h"
#include "Int_motor.h"

#define SPEED_LIMIT   1000

/* 本地控制目标速度（协议码，-1000~1000） */
static volatile int16_t s_target_speed = 0;
/* 本地控制是否生效 */
static volatile uint8_t s_active = 0U;
/* 强制停机锁存：STOP 后为真，忽略任何外部目标，直到再次 START */
static volatile uint8_t s_stop_latch = 0U;
/* 急停/异常停机锁存：优先级最高，需显式复位(RESET/START)才能重新启动 */
static volatile uint8_t s_estop_latch = 0U;

void App_MotorControl_Init(void)
{
    s_target_speed = 0;
    s_active = 0U;
    s_stop_latch = 0U;
    s_estop_latch = 0U;
    Int_Motor_Stop();
}

void App_MotorControl_SetSpeed(int16_t speed)
{
    if (speed > SPEED_LIMIT)
    {
        speed = SPEED_LIMIT;
    }
    else if (speed < -SPEED_LIMIT)
    {
        speed = -SPEED_LIMIT;
    }

    /* 一次新的启动（重新启动）：清除停机与急停锁存 */
    s_stop_latch = 0U;
    s_estop_latch = 0U;

    s_target_speed = speed;
    s_active = 1U;
}

/* 急停：立即停机并锁存，忽略一切目标，直到显式复位 */
void App_MotorControl_EMStop(void)
{
    s_estop_latch = 1U;
    s_target_speed = 0;
    s_active = 0U;
    s_stop_latch = 0U;
    Int_Motor_Stop();
}

uint8_t App_MotorControl_IsEMLatched(void)
{
    return s_estop_latch;
}

/* 重新启动入口：释放急停锁存，回归空闲/CAN 模式 */
void App_MotorControl_ReleaseEMStop(void)
{
    s_estop_latch = 0U;
    s_target_speed = 0;
    s_active = 0U;
    Int_Motor_Stop();
}

void App_MotorControl_Stop(void)
{
    s_target_speed = 0;
    s_active = 0U;
    s_stop_latch = 1U;
    Int_Motor_Stop();
}

uint8_t App_MotorControl_IsActive(void)
{
    return s_active;
}

int16_t App_MotorControl_GetTargetSpeed(void)
{
    return s_target_speed;
}

uint8_t App_MotorControl_IsStopLatched(void)
{
    return s_stop_latch;
}

void App_MotorControl_ReleaseStopLatch(void)
{
    s_stop_latch = 0U;
    s_active = 0U;
    s_target_speed = 0;
    Int_Motor_Stop();
}