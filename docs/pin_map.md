# Mini VCU 引脚分配表

## 1. F407 主控

| 功能 | 外设/引脚 | 用途 | 备注 |
|---|---|---|---|
| CAN1_RX | `PA11` | 接收 CAN 总线数据 | `AF9_CAN1`，250 kbit/s |
| CAN1_TX | `PA12` | 发送 CAN 总线数据 | `AF9_CAN1`，250 kbit/s |
| USART1_TX | `PA9` | 调试串口发送 | 115200 8N1 |
| USART1_RX | `PA10` | 调试串口接收 | 115200 8N1 |
| I2C1_SCL | `PB6` | AHT20 时钟 | |
| I2C1_SDA | `PB7` | AHT20 数据 | 器件地址 `0x38 << 1` |
| ADC1_IN0 | `PA0` | 12 位 ADC 采样 | 电压换算 `raw * 3.3 / 4095` |
| LED | `PA5` | 状态指示 | 推挽输出，500 ms 翻转 |
| SWDIO | `PA13` | ST-Link/SWD 调试 | |
| SWCLK | `PA14` | ST-Link/SWD 调试 | |
| PWMA | `PB0` | 电机速度 PWM | `TIM3_CH3`，1 kHz，CCR=0~1000 |
| AIN1 | `PB12` | 电机方向控制 1 | GPIO 推挽输出 |
| AIN2 | `PB13` | 电机方向控制 2 | GPIO 推挽输出 |
| STBY | `PB14` | 电机驱动使能 | 高电平使能，由 `Int_Motor_Init()` 置高 |
| 编码器 A 相 | `PC6` | 正交编码器输入 | `TIM8_CH1`，`AF3_TIM8` |
| 编码器 B 相 | `PC7` | 正交编码器输入 | `TIM8_CH2`，`AF3_TIM8` |

注：`PA6`（`TIM3_CH1`）在 CubeMX 中也配置了 PWM，但电机驱动只使用 `PB0`，不要误接。

## 2. F103 节点

| 功能 | 外设/引脚 | 用途 | 备注 |
|---|---|---|---|
| CAN_RX | `PB8` | 接收 CAN 总线数据 | CAN1 重映射，250 kbit/s |
| CAN_TX | `PB9` | 发送 CAN 总线数据 | CAN1 重映射，250 kbit/s |
| SWDIO | `PA13` | SWD 调试/下载 | |
| SWCLK | `PA14` | SWD 调试/下载 | |