# Mini VCU 开发记录

## Day 1：屏幕接入前准备

### 当前硬件
- 主控：STM32F407VGT6
- 从节点：STM32F103C8T6
- 显示屏：2.4 寸 SPI TFT
- LCD 控制器：ILI9341
- 触摸控制器：XPT2046
- LCD 分辨率：240 × 320
- 屏幕带 SD 卡座

### 已有功能
- CAN 通信
- FreeRTOS
- ADC 采样
- AHT20 温湿度读取
- 电机 PWM 控制
- 编码器测速
- PI/PID 速度闭环控制

### Day 1 目标
- [ ] 备份原始 Keil 工程
- [ ] 确认副本能够正常编译
- [ ] 确认 LCD / Touch 引脚定义
- [ ] 完成屏幕接线规划