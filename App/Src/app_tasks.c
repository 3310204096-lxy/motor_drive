/**
  ******************************************************************************
  * @file    app_tasks.c
  * @brief   Application tasks for the dual-sensor motor control platform.
  *          Current stage: P1 - read both sensors and report over UART.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>

#include "cmsis_os.h"

#include "i2c.h"

#include "app_tasks.h"
#include "sensor.h"
#include "encoder.h"
#include "as5600.h"

/* USER CODE BEGIN 0 */

/* ---------- 任务周期配置 --------------------------------------------------------------------------
 * 调度节拍由 SysTick 提供（configTICK_RATE_HZ = 1000，即 1 tick = 1 ms）。
 * 所有周期都必须是 tick 的整数倍。
 * ------------------------------------------------------------------------------------------------ */
#define SENSOR_PERIOD_MS      10U    /* 霍尔采样周期                                     */
#define REPORT_PERIOD_MS      20U    /* 磁编码器采样周期                                 */
#define PRINT_DIVIDER         10U    /* 20 ms x 10 = 200 ms 上报一次                     */

/* 最近一次磁编码器采样结果，供上报任务使用 */
static Sensor_Data_t s_mag;
static uint16_t      s_mag_raw = 0U;

/* Private function prototypes -----------------------------------------------*/
static const char *P1_FlagStr(uint8_t flags);
static void        P1_PrintStatus(void);

/* USER CODE END 0 */

/**
  * @brief  霍尔编码器采样任务。
  * @param  argument 未使用
  * @retval None
  * @note   TIM2 在硬件层累加计数，本任务只负责按固定节拍取快照并累加。
  *         16 位计数器回绕由 Encoder_Update() 内的 int16_t 差分处理。
  */
void App_SensorTask(void *argument)
{
  uint32_t next = osKernelGetTickCount();

  (void)argument;

  for (;;)
  {
    next += SENSOR_PERIOD_MS;
    osDelayUntil(next);

    Encoder_Update();

    /* 若被更高优先级任务长时间占用导致节拍严重滞后，
       重新对齐基准，避免连续追赶式调度。                       */
    if ((int32_t)(osKernelGetTickCount() - next) >= (int32_t)SENSOR_PERIOD_MS)
    {
      next = osKernelGetTickCount();
    }
  }
}

/**
  * @brief  磁编码器采样与上报任务。
  * @param  argument 未使用
  * @retval None
  * @note   AS5600 走 I2C，单次读取在 400 kHz 下约 150 us，属于阻塞式调用，
  *         因此单独放在低优先级任务里，不占用控制环的时间片。
  */
void App_ReportTask(void *argument)
{
  uint32_t next = osKernelGetTickCount();
  uint32_t divider = 0U;

  (void)argument;

  for (;;)
  {
    next += REPORT_PERIOD_MS;
    osDelayUntil(next);

    (void)AS5600_Sample(&s_mag, &s_mag_raw);

    if (++divider >= PRINT_DIVIDER)
    {
      divider = 0U;
      P1_PrintStatus();
    }

    if ((int32_t)(osKernelGetTickCount() - next) >= (int32_t)REPORT_PERIOD_MS)
    {
      next = osKernelGetTickCount();
    }
  }
}

/* USER CODE BEGIN 1 */

/**
  * @brief  把故障标志转换为可读字符串。
  */
static const char *P1_FlagStr(uint8_t flags)
{
  if (flags == SENSOR_FLAG_NONE)                { return "OK";     }
  if ((flags & SENSOR_FLAG_COMM_ERROR) != 0U)   { return "COMM";   }
  if ((flags & SENSOR_FLAG_NO_MAGNET) != 0U)    { return "NOMAG";  }
  if ((flags & SENSOR_FLAG_WEAK_FIELD) != 0U)   { return "WEAK";   }
  if ((flags & SENSOR_FLAG_STRONG_FIELD) != 0U) { return "STRONG"; }
  return "UNKNOWN";
}

/**
  * @brief  打印自检表头，说明当前使用的换算基准。
  */
void App_PrintHeader(void)
{
  printf("\r\n==================================================\r\n");
  printf(" Motor Drive - P1 Sensor Checkout\r\n");
  printf(" Build : %s %s\r\n", __DATE__, __TIME__);
  printf(" Hall  : TIM2 encoder, %u PPR x 4 = %u CPR, gear 1:%u\r\n",
         (unsigned int)ENCODER_PPR,
         (unsigned int)ENCODER_CPR,
         (unsigned int)ENCODER_GEAR_RATIO);
  printf(" Mag   : AS5600 12-bit, I2C1 @ %lu Hz, addr 0x36\r\n",
         (unsigned long)hi2c1.Init.ClockSpeed);
  printf(" RTOS  : CMSIS-RTOS v2, tick = %u Hz\r\n",
         (unsigned int)osKernelGetTickFreq());
  printf("==================================================\r\n\r\n");
}

/**
  * @brief  打印一次两路传感器的当前读数。
  * @note   刻意不使用 %f——newlib-nano 默认不链接浮点格式化，
  *         这里手工拆整数与小数部分，兼顾体积与可读性。
  */
static void P1_PrintStatus(void)
{
  int32_t  hall     = Encoder_GetCount();
  float    hall_deg = (float)hall * 360.0f / (float)ENCODER_CPR;
  int32_t  hd10     = (int32_t)(hall_deg * 10.0f);
  int32_t  hd_i     = hd10 / 10;
  int32_t  hd_f     = hd10 % 10;

  float    mag_deg  = s_mag.Angle * 57.295780f;
  int32_t  md10     = (int32_t)(mag_deg * 10.0f);
  int32_t  md_i     = md10 / 10;
  int32_t  md_f     = md10 % 10;

  /* 负数取模为负，单独取绝对值，避免打印成 "87.-3" */
  if (hd_f < 0) { hd_f = -hd_f; }
  if (md_f < 0) { md_f = -md_f; }

  printf("t=%6lu  H=%8ld (%6ld.%01ld deg)  M=%4u (%5ld.%01ld deg)  %s%s\r\n",
         (unsigned long)osKernelGetTickCount(),
         (long)hall,
         (long)hd_i, (long)hd_f,
         (unsigned int)s_mag_raw,
         (long)md_i, (long)md_f,
         P1_FlagStr(s_mag.Flags),
         (s_mag.Valid != 0U) ? "" : " INVALID");
}

/* USER CODE END 1 */
