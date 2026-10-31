/**
  ******************************************************************************
  * @file    app_main.c
  * @brief   Application layer entry: peripheral setup and task creation for
  *          the dual-sensor motor control platform.
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

#include "main.h"
#include "usart.h"

#include "app_main.h"
#include "app_tasks.h"
#include "encoder.h"
#include "as5600.h"

/* USER CODE BEGIN 0 */

/* ---------- 任务属性 ------------------------------------------------------------------------------
 * stack_size 单位是字节。SensorTask 只做寄存器读取，ReportTask 需要调用
 * printf，栈要留足（newlib 的格式化函数本身占用 200~400 字节）。
 * ------------------------------------------------------------------------------------------------ */
static const osThreadAttr_t s_sensorTaskAttr =
{
  .name       = "SensorTask",
  .stack_size = 256U * 4U,   /* 1024 Bytes */
  .priority   = (osPriority_t)osPriorityAboveNormal,
};

static const osThreadAttr_t s_reportTaskAttr =
{
  .name       = "ReportTask",
  .stack_size = 384U * 4U,   /* 1536 Bytes */
  .priority   = (osPriority_t)osPriorityLow,
};

/* USER CODE END 0 */

/**
  * @brief  重定向 printf 到 USART1。
  * @param  ch 待发送字符
  * @retval 原字符
  * @note   逐字节阻塞发送。P1 阶段每 200 ms 才输出一行，开销可以忽略；
  *         后续接入上位机协议时改为环形缓冲 + 中断/DMA 发送。
  */
int __io_putchar(int ch)
{
  uint8_t c = (uint8_t)ch;

  (void)HAL_UART_Transmit(&huart1, &c, 1U, 100U);
  return ch;
}

/**
  * @brief  初始化应用外设并创建业务任务。
  * @note   由 FreeRTOS 的 StartDefaultTask 调用。返回后该任务自行退出，
  *         调度权交给下面创建的各个任务。
  * @retval None
  */
void App_Init(void)
{
  osThreadId_t sensorHandle;
  osThreadId_t reportHandle;

  /* 1. 外设初始化（此时调度器已运行，但本任务优先级最高，不会被抢占） */
  Encoder_Init();

  App_PrintHeader();

  if (AS5600_Init() == 0U)
  {
    printf("[P1] AS5600 no response, check I2C wiring / pull-ups\r\n");
  }
  else
  {
    printf("[P1] AS5600 online\r\n");
  }
  printf("[P1] Turn the motor shaft by hand at a steady speed.\r\n\r\n");

  /* 2. 创建业务任务 */
  sensorHandle = osThreadNew(App_SensorTask, NULL, &s_sensorTaskAttr);
  reportHandle = osThreadNew(App_ReportTask, NULL, &s_reportTaskAttr);

  if ((sensorHandle == NULL) || (reportHandle == NULL))
  {
    /* 堆内存不足或属性非法，无法继续运行 */
    Error_Handler();
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
