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

---

# 附录：4-4-2 电机控制页（本地直控，方案1）

## 一、目标与架构决策

在 F407 的 LCD 控制页上直接控制电机。回顾现有链路：**F103 是发送端（发 0x301）、F407 是主控兼电机驱动（收 0x301 → PID → TB6612）、并回发 0x302 反馈**。而 LCD 也挂在 F407 上，因此采用**本地直控（方案1）**：控制页按钮写入本地目标速度，PID 任务以本地目标为源，而非走 CAN 自环。这样复用原有校验/PI/PWM/故障逻辑，最安全。

真实电机状态由 PID 任务维护的 `g_ui_target_rpm / g_ui_actual_rpm / g_ui_pwm` 提供，控制页据此刷新，不再只是"命令已发送"。

## 二、目标速度来源优先级

```
强制停机锁存(STOP后)  >  本地控制(控制页START)  >  F103 CAN 帧
```

- 首次上电：锁存=0、本地未生效 → 纯 CAN 链路，行为与 Day13 一致（无回归）。
- 本地 START：清除锁存、进入本地控速。
- 本地 STOP：置锁存，电机强制停；之后 CAN 无法再驱动，直到再次手动 START 或点 `CAN MODE` 释放。

## 三、涉及文件与改动

| 文件 | 改动 |
|---|---|
| `Application/App_MotorControl.c/.h` | 增加本地目标速度、`s_active`（生效标志）、`s_stop_latch`（强制停机锁存）；新增 `IsActive` / `GetTargetSpeed` / `IsStopLatched` / `ReleaseStopLatch`。此前是其**死代码**，现被控制页正式引用。 |
| `Core/Src/freertos.c` | PID 任务目标源改为三优先级（锁存→本地→CAN）；`APP_LCD_Test_Task` 每 200ms 调 `LCD_UI_UpdateControlStatus` 刷新控制页真实状态。 |
| `Application/App_lcd_ui.c/.h` | 重写控制页：调速 `-`/`+`、`START`/`STOP`、`CAN MODE`、`TX` 结果、`TGT/RUNNING/ACT/PWM` 真实状态；主页电机状态联动真实运行状态。新增 `LCD_UI_UpdateControlStatus()`。 |

## 四、控制页交互

- `+`/`-`：每步 10 RPM，限 安全范围 ±150 RPM；运行中点击会**实时调速**。
- `START`：`App_MotorControl_SetSpeed(本地目标)` 使能 + 回发 0x302 `SET_ACCEPTED` 反馈帧，`TX` 显示回发结果（OK/HOLD/FAIL）。
- `STOP`：`App_MotorControl_Stop()` 停机并**置锁存** + 回发 0x302 `STOP_ACCEPTED`。
- `CAN MODE`：`App_MotorControl_ReleaseStopLatch()` 清锁存、退出本地控制，交还 F103。
- 状态卡：`TGT`（目标 RPM）、`MOTOR: RUNNING/STOPPED`（真实驱动状态，绿色/灰色）、`ACT + PWM`（编码器实际转速与 PWM 占空比）。

## 五、关键实现要点

- 换算：协议码 `-1000~1000` ↔ RPM，`UI_MOTOR_MAX_RPM = 169`（与 `freertos.c` 的 `MOTOR_MAX_RPM` 对应）。
- 主页动量联动：`LCD_UI_UpdateSpeed()` 依据真实 PWM 覆盖主页电机状态标签，CAN 与本地控制驱动均适用。
- 锁存语义：`Stop()` 置锁存 → PID 强制 `target_rpm=0`、清超时故障并忽略一切外部目标；`SetSpeed`（下次 START）清锁存。

## 六、安全边界

- 目标转速限制 ±150 RPM（协议码约 ±888，在合法 ±1000 内）。
- 本地控制生效期间清除 `FAULT_CAN_TIMEOUT`；PI 限幅与渐变逻辑沿用原有实现，无新增失控点。
- STOP 后锁存保证电机保持停转，杜绝 F103 残留帧误启动。

## 七、验收建议清单

- [ ] `START` 后电机按设定 RPM 转动，`MOTOR: RUNNING`、`ACT/PWM` 实时更新
- [ ] `+`/`-` 运行中可实时调速，且不超 ±150 RPM 边界
- [ ] `STOP` 后电机停转，即使 F103 持续发 SET 帧也不重启（锁存生效）
- [ ] `CAN MODE` 释放锁存后，F103 CAN 帧可恢复驱动
- [ ] `TX` 正确显示反馈帧回发结果（挂载/未挂载 CAN 总线时 OK/FAIL）
- [ ] 主页电机状态随真实驱动状态联动
- [ ] 回归：CAN 接收、PID、ADC/AHT20、UART 打印不受影响

## 八、已知事项

- `CAN MODE` 是释放锁存、回归 CAN 的唯一入口；若只按 `START` 则进入的是本地控速而非纯 CAN。
- 主页 `TGT/PWM` 挤在一行、控制页底部队齐等 UI 瑕疵已记录，留待"下一轮 UI 美化"统一处理。
- 工程为 Keil MDK（`.uvprojx`），需重新编译 `F407_Main` 使 `App_MotorControl` 正常链接。