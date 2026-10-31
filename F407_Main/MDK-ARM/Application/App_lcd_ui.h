#ifndef __APP_LCD_UI_H
#define __APP_LCD_UI_H

#include <stdint.h>
#include <stdio.h>
#include "App_Fault.h"

void LCD_UI_ShowHomePage(void);
void LCD_UI_ShowControlPage(void);

void LCD_UI_UpdateCAN(uint32_t can_id,
                      uint8_t dlc,
                      const uint8_t data[8],
                      uint32_t rx_count,
                      uint8_t can_online);

void LCD_UI_UpdateMotor(uint32_t can_id,
                        uint8_t dlc,
                        const uint8_t data[8]);

void LCD_UI_UpdateSpeed(float target_rpm,
                        float actual_rpm,
                        int16_t pwm);

void LCD_UI_UpdateFault(uint32_t fault_flags);

/* 控制页：用真实电机运行数据刷新状态 */
void LCD_UI_UpdateControlStatus(float actual_rpm, int16_t pwm);

#endif