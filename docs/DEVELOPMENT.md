# motor_drive 开发手册

> 本文件是项目的**活文档**：每定下一个架构决策、每踩一个坑，都往这里追加。
> 目的是让后来接手的人（或几个月后的自己）不用重新摸索一遍。
>
> 最后更新：2026-11-06

---

## 1. 项目概况

**便携式计算机控制实验系统** —— 电机调速 + 温度控制双回路实验平台。

| 项 | 内容 |
|---|---|
| 被控对象 | 直流伺服电机（12V / 80mA 额定）+ 加热铜块（12V / 5W~12W） |
| 主控 | STM32F103C8T6（LQFP-48，64KB Flash / 20KB RAM） |
| 电机驱动 | DRV8871（H 桥，限流 1.5A） |
| 加热驱动 | AO3400 N-MOS 低边 PWM |
| 温度 | DS18B20 单总线（铜块内置） |
| 位置反馈 | 光电编码器 A/B 相（TIM2）+ AS5600 磁编码器（I2C） |
| 通信 | USART1 115200，与上位机 Measurement Studio 通信 |
| 采样 | 5 路模拟量：电机电压/电流、加热电压/电流、温度 |

**硬件接口的完整定义见 [`hardware_interface.md`](./hardware_interface.md)**，引脚、换算公式、接口定义都在那里。

**方案设计见 [`v2_final_plan.html`](./v2_final_plan.html)**。

---

## 2. 开发环境

| 组件 | 版本 / 说明 |
|---|---|
| IDE | VSCode + CMake Tools |
| 工具链 | arm-none-eabi-gcc（通过 CMake 调用） |
| 构建系统 | CMake（`CMakePresets.json` 定义 Debug/Release） |
| 配置工具 | STM32CubeMX 6.15.0，固件包 FW_F1 V1.8.7 |
| RTOS | FreeRTOS（CMSIS-V2 封装） |
| 原理图 | 嘉立创 EDA 专业版（云端工程 `motor_drive`） |

**编译**：VSCode 里直接 Build，或在 `build/` 目录下 `cmake --build .`。

**注意**：不要在远程/沙箱环境跑全量编译，慢且容易误判。本地 VSCode 几分钟就好。

### 2.1 构建与烧录（统一方案）

**不再为每个项目单独写 task。** 通用逻辑放在用户目录，项目里只引用。

```
%USERPROFILE%\.stm32\
├── stm32.ps1          主脚本：build / flash / clean / rebuild / config
└── init-project.ps1   给新项目生成 .vscode
```

**项目里的 `.vscode/tasks.json` 只有五个任务，内容完全一样，不写任何设备型号或工具路径**：

| 任务 | 快捷键 / 触发 | 说明 |
|---|---|---|
| 编译 | `Ctrl+Shift+B` | 增量编译，输出到 `build/Debug/` |
| 烧录 | 任务面板 | 编译 + 下载 |
| 清理 | 任务面板 | 删除 `build/Debug/` |
| 重新编译 | 任务面板 | clean + configure + build |
| 编译并烧录 | 任务面板 | 前两者串行 |

**脚本自动处理的三件事**：

1. **找最新版工具链**。ST 扩展装在版本号目录里（`stm32cube-ide-build-cmake-1.47.0-win32-x64`），扩展一更新路径就变。脚本遍历取版本号最大的那个。
2. **烧录器自动降级**。优先 ST-LINK（`STM32_Programmer_CLI`），连不上自动切 J-Link；也可用 `-Flasher jlink|stlink` 强制指定。
3. **MCU 型号从 `.ioc` 读**。抓 `Mcu.CPN=STM32F103C8T6`，J-Link 需要的基础型号（`STM32F103C8`）由脚本自动裁剪，不用手填。

**新项目初始化**：

```powershell
cd E:\code_source\new_project
& "$env:USERPROFILE\.stm32\init-project.ps1"
```

生成 `tasks.json` + `settings.json`，并顺手把 `build/` 加进 `.gitignore`。已有文件不会覆盖，除非加 `-Force`。

**直接命令行调用**（不经过 VSCode）：

```powershell
# 在项目根目录
& "$env:USERPROFILE\.stm32\stm32.ps1" build
& "$env:USERPROFILE\.stm32\stm32.ps1" flash -Preset Release
& "$env:USERPROFILE\.stm32\stm32.ps1" clean
```

**踩坑提醒**：这两个 `.ps1` 必须存为 **UTF-8 with BOM**。PowerShell 5.1 读无 BOM 的 UTF-8 会按 GBK 解码，中文全变乱码，严重时解析直接报语法错误。`init-project.ps1` 里有中文任务名，尤其要注意。改完文件用这条命令补 BOM：

```powershell
$f = "$env:USERPROFILE\.stm32\init-project.ps1"
$s = [System.IO.File]::ReadAllText($f, (New-Object System.Text.UTF8Encoding($false)))
[System.IO.File]::WriteAllText($f, $s, (New-Object System.Text.UTF8Encoding($true)))
```

---

## 3. 代码结构

```
motor_drive/
├── App/                    ← 应用层（我们写的代码）
│   ├── Inc/                ← 头文件
│   └── Src/                ← 源文件
│       └── app_main.c      ← 应用入口，由 StartDefaultTask 调用
├── Core/                   ← CubeMX 生成的 HAL 层
│   ├── Inc/                ← main.h / 各外设 .h
│   └── Src/
│       ├── main.c
│       ├── adc.c / tim.c / gpio.c / i2c.c / usart.c / freertos.c
│       └── stm32f1xx_it.c  ← 中断服务函数
├── Drivers/                ← CMSIS + HAL 库（不要手改）
├── Middlewares/            ← FreeRTOS（不要手改）
├── docs/                   ← 设计文档、参考手册
└── CMakeLists.txt          ← 构建配置，新增 .c 文件要在这里登记
```

**规则**：

- `Core/` 里的代码由 CubeMX 生成，**只在 `USER CODE BEGIN/END` 之间写自己的内容**
- `App/` 里随便写，不受 CubeMX 重新生成影响
- 新增源文件必须加到 `CMakeLists.txt` 的 `target_sources`

---

## 4. Git 分支策略

```
main                    ← 稳定基线，任何时候都能编译、能烧录
  └─ develop            ← 集成分支，日常开发在这
       ├─ feat/adc      ← 单模块分支，从 develop 拉
       ├─ feat/encoder
       ├─ feat/pwm
       ├─ feat/temp
       ├─ feat/mag-enc
       └─ feat/uart
```

### 4.1 单模块开发流程

```bash
# 1. 从 develop 拉分支
git checkout develop
git checkout -b feat/adc

# 2. 写代码 + 烧录验证（这一步必须真机跑通）

# 3. 提交（只提交源码，不要 git add -A，构建产物会污染）
git add App/Src/adc_app.c App/Inc/adc_app.h CMakeLists.txt
git commit -m "feat(adc): 四通道 DMA 循环采集 + 电压换算"

# 4. 合并回 develop（--no-ff 保留分支痕迹）
git checkout develop
git merge --no-ff feat/adc

# 5. 删除已合并的分支
git branch -d feat/adc
```

### 4.2 为什么这么分

- **单模块独立验证**：ADC 的 bug 不会污染编码器的代码，出了问题范围明确
- **`--no-ff` 保留历史**：`git log --graph` 能看出每段对应哪个模块，回溯方便
- **`main` 保持稳定**：随时可以回退到一个"能跑"的状态

### 4.3 提交信息格式

```
<类型>(<模块>): <简述>

类型：feat / fix / refactor / docs / cube / chore
```

例：
- `feat(pwm): TIM3 CH3 输出 20kHz 可调占空比`
- `cube: PA4 改开漏、ADC1 四通道+DMA`
- `fix(encoder): 修复计数溢出导致的转速跳变`

---

## 5. 开发顺序与状态

按"独立性从高到低"排，前面的模块出问题时不会牵连后面的。

| # | 模块 | 分支 | 验证方式 | 状态 |
|---|---|---|---|---|
| 1 | ADC 四路采集 | `feat/adc` | 万用表对照电压值 | ⬜ 未开始 |
| 2 | 光电编码器 | `feat/encoder` | 手转电机看计数值增减 | ⬜ 未开始 |
| 3 | PWM 输出 | `feat/pwm` | 示波器/万用表看占空比 | ⬜ 未开始 |
| 4 | DS18B20 测温 | `feat/temp` | 读温度对比室温 | ⬜ 未开始 |
| 5 | AS5600 磁编码器 | `feat/mag-enc` | I2C 扫描到 0x36 并读到角度 | ⬜ 未开始 |
| 6 | 串口协议 | `feat/uart` | 上位机收到数据包 | ⬜ 未开始 |
| 7 | 软件架构整理 | `arch/framework` | 全部模块合并后重构 | ⬜ 未开始 |

**为什么这个顺序**：

- ADC 最独立，纯输入，验证方法明确（万用表）
- 编码器只读计数器，不涉及输出
- PWM 是纯输出，可以先不管反馈
- DS18B20 时序最难，放在有调试经验之后
- 串口协议要等各模块有数据可发
- 架构最后做，因为**先有能跑的东西，才知道该怎么抽象**

---

## 6. 各模块验证方法

每个模块都必须**单独烧录、单独验证**，不能"写完一起测"。

### 6.1 ADC

```c
// 启动 DMA 循环转换
HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf, 4);

// 读四路
V_motor = (adc_buf[0] * 3.3f / 4095.0f - 1.65f) / 0.13f;
I_motor = adc_buf[1] * 3.3f / 4095.0f / 100.0f / 0.01f;
I_heat  = adc_buf[2] * 3.3f / 4095.0f / 100.0f / 0.01f;
V_drain = adc_buf[3] * 3.3f / 4095.0f * 4.0f;
V_heat  = 12.0f - V_drain;
```

**验证**：万用表量 PA2 对地电压，应该等于 `1.65V`（电机静止、输出 0V 时）。若不是 1.65V，检查差分放大和基准。

### 6.2 编码器

```c
HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
int16_t cnt = (int16_t)TIM2->CNT;
```

**验证**：手转电机轴一圈，`cnt` 变化量应等于 `线数 × 4`。正转增、反转减。

### 6.3 PWM

```c
HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 1800);   // 50%
```

**验证**：万用表直流档量 PB0 对地，50% 占空比应该读到约 1.65V（平均值）。

### 6.4 DS18B20

**验证**：读出的温度应该接近室温（20~25°C）。差太多说明时序不对。

### 6.5 AS5600

```c
// 先扫描 I2C 总线
for (uint8_t a = 1; a < 128; a++) {
    if (HAL_I2C_IsDeviceReady(&hi2c1, a << 1, 3, 10) == HAL_OK)
        printf("found 0x%02X\r\n", a);
}
// AS5600 应该是 0x36
```

---

## 7. 软件开发踩坑记录

> 只记录**软件开发阶段**的问题：编译、运行时、架构。
> CubeMX 配置、EDA 画图这类一次性的配置错误不在此列（那些在配置时改对就完了，不会反复踩）。

### 7.1 构建与工具链

| 现象 | 原因 | 解法 |
|---|---|---|
| 编译找不到新增的 .c | 没在 `CMakeLists.txt` 的 `target_sources` 里登记 | 加一行路径 |
| CubeMX 重新生成后自定义代码丢失 | 写在了 `USER CODE BEGIN/END` 之外 | 所有自己的代码必须放在标记区间内 |
| `printf` 没有任何输出 | 没实现 `__io_putchar`，或浮点格式化没开 | 重定向 + 链接加 `-u _printf_float` |
| 改了头文件不生效 | 增量编译缓存 | Clean 后重新 Build |

### 7.2 运行时

（待补充 —— 每解决一个 bug 就在这里加一行，写清"现象 / 原因 / 解法"）

### 7.3 架构决策

（待补充 —— 每次定下一个架构选择，记录"为什么这么选"和"放弃了什么方案"）

## 8. 待决事项

- [ ] **PA6/PA7 的 PWM 用法**：目前是 TIM3_CH1/CH2 输出到 DRV8871 的 IN1/IN2。调速时把 PWM 打在其中一个、另一个拉低；换向时要注意死区，避免上下桥直通。
- [ ] **加热 PWM 与电机 PWM 共用 TIM3**：频率都是 20kHz，无法单独设置。如果以后要改加热频率，需要把加热挪到别的定时器。
- [ ] **两颗 INA180 输出没有 RC 滤波**：20kHz PWM 纹波会进 ADC，建议各加 1kΩ + 100nF。软件上可以先用滑动平均顶一下。

---

## 9. 更新日志

| 日期 | 内容 |
|---|---|
| 2026-11-06 | 建立文档。清空旧代码只留 `app_main` 骨架，确定分支策略和开发顺序 |
| 2026-11-06 | CubeMX 重配完成：ADC 四通道 + DMA、TIM2 编码器修正、PA4 改开漏 |
| 2026-11-06 | 修正 I2C 用途（磁编码器，非 OLED）；踩坑记录只保留软件开发相关 |

---

## 附录：常用命令

```bash
# 查看分支图
git log --graph --oneline --all -20

# 看某个文件的历史
git log --oneline -- docs/hardware_interface.md

# 撤销工作区改动（危险，会丢未提交内容）
git checkout -- <file>

# 临时保存当前改动
git stash push -m "wip: adc 调试中"
git stash pop
```
