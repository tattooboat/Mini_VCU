# Mini VCU 屏幕开发记录 — Day 2

## 一、开发环境与目标
- 主控：STM32F407VGT6
- 屏幕：2.4 寸 ILI9341，320×240 横屏
- 工具：STM32CubeMX、Keil、HAL、FreeRTOS
- LCD 通信：GPIO 模拟 SPI
- 目标：完成基础图形、ASCII 字符显示和 Mini VCU 仪表主页。

## 二、完成内容

### 1. 基础图形
在 `lcd_ili9341.h/.c` 中实现并验证：
- `LCD_DrawPoint()`：绘制像素
- `LCD_FillRect()`：实心矩形
- `LCD_DrawRect()`：空心矩形
- `LCD_FillColor()`：全屏填色

测试结果：图形颜色与位置均符合预期。

### 2. ASCII 字符显示
新增 `lcd_font.h`，采用 5×7 点阵字体，支持常见大写字母、数字和部分符号；小写字母会转换成大写，未收录字符显示问号。

增加接口：
- `LCD_DrawChar()`：显示单个字符
- `LCD_ShowString()`：显示字符串
- `LCD_ShowStringScale()`：可选的整数倍放大字符串接口

测试结果：`MINI VCU`、`CAN: OK`、`SPEED: 120`、`MOTOR: STOP` 等文字显示正常。当前决定暂时不再调整字号。

### 3. Mini VCU 仪表主页
新增 `App_lcd_ui.h/.c`，主页包含：
- 顶部标题栏：`MINI VCU`、`SYSTEM READY`
- 左侧速度面板：`VEHICLE SPEED`、演示值 `120 km/h`
- 右侧电机面板：`MOTOR STATUS`、`STOP`、`PWM: 0%`
- CAN 区域：`CAN BUS: READY`、`MCU: F407`
- 底部信息栏：`MINI VCU DEMO`

`freertos.c` 中的 `LCD_Test_Task()` 调用 `LCD_UI_ShowHomePage()`，主页绘制一次后进入循环等待。

测试结果：主页显示正常，布局和文字完整。

## 三、关键文件
```text
USER/
├── lcd_ili9341.h
├── lcd_ili9341.c
├── lcd_font.h
├── App_lcd_ui.h
└── App_lcd_ui.c
```
目录可以不同，但 `.c` 文件必须加入 Keil 工程，头文件路径必须正确，避免同一源文件重复加入。

## 四、硬件连接记录

| LCD 引脚 | STM32F407 引脚/连接 |
|---|---|
| SCK | PC0 |
| SDI / MOSI | PC1 |
| CS | PC3 |
| D/C | PC4 |
| RESET | PC5 |
| LED | 当前接 3.3V，背光已点亮 |
| GND | GND |
| VCC | 当前接 5V，模块供电实测约 5V |
| SDO / MISO | 暂未连接 |
| 触摸相关引脚 | 暂未连接 |

注意：后续应根据模块原理图或卖家资料确认逻辑电平和背光限流设计；不要仅凭背光点亮就认定所有引脚都可直接接 5V。

## 五、当前实现注意事项
1. `LCD_Init()` 只初始化一次，放在 FreeRTOS 调度器启动之前。
2. `LCD_Test_Task()` 调用 `LCD_UI_ShowHomePage()` 后只等待，不持续整屏重绘。
3. `120 km/h`、`PWM: 0%`、`CAN BUS: READY` 都是静态演示内容，不代表真实车辆数据或 CAN 状态。
4. 当前字体不支持中文。
5. 字符绘制采用逐像素方式，已满足当前测试；后续可优化为窗口批量写入。
6. 触摸控制器尚未接线和验证；本阶段只完成显示功能。

## 六、Day 2 验收清单
- [x] 基础图形显示通过
- [x] ASCII 字符显示通过
- [x] Mini VCU 主页显示通过
- [x] FreeRTOS 任务中显示主页
- [x] 暂不调整字体大小
- [ ] 接入真实 CAN 数据
- [ ] 显示 CAN 超时/故障状态
- [ ] 测试触摸控制器
- [ ] 后续评估 LVGL 迁移

## 七、后续步骤：Day 3 — 接入真实 CAN 数据

### 第 1 步：备份工程
先确认当前 LCD 主页可正常运行，复制一份工程备份。后续改动只在开发副本中进行。

### 第 2 步：梳理 CAN 协议
核对 `App_can_protocol.c/.h`、`Int_can.c/.h`、`App_MotorCan.c/.h`：
- CAN ID 分别代表什么
- DLC 和每个数据字节的含义
- F407 接收后由哪个函数解析
- 哪些现有变量或 getter 可安全读取

### 第 3 步：建立 UI 数据接口
先确定字段及其真实来源，例如 CAN 是否超时、电机运行状态、目标转速、实际转速、PWM。没有确认来源的字段先不要接入。

### 第 4 步：先显示真实 CAN 状态
优先将静态 `CAN BUS: READY` 替换为基于有效接收时间的在线/超时状态。超时后应显示 `CAN TIMEOUT` 或 `OFFLINE`，而不是继续显示 READY。

### 第 5 步：接入电机状态
根据现有项目的真实变量更新电机状态、目标转速、实际转速或 PWM。若没有车速传感器，不要把电机 RPM 直接标成 km/h。

### 第 6 步：任务中刷新 UI
不要在 CAN 接收中断回调中直接绘图。中断负责收帧和更新协议数据，LCD 在 FreeRTOS 任务上下文刷新。共享数据时应使用项目已有的线程安全机制，避免读到不一致的值。

### 第 7 步：回归测试
确认 LCD 更新不会影响 CAN 接收、电机控制、ADC/AHT20 传感器任务及 UART 调试输出。避免每个周期整屏刷新，先定期更新数据区域。

## 八、总结
Day 2 已完成 LCD 基础绘图、ASCII 字符显示和 Mini VCU 静态仪表主页，并已上板验证。下一阶段重点是将现有 CAN/电机控制状态安全接入 UI，增加通信超时提示，并确保不影响已有控制任务。
