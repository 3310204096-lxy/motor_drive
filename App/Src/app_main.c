/**
  ******************************************************************************
  * @file    app_main.c
  * @brief   应用层入口。
  *
  *          当前阶段：仅保留启动框架，各功能模块按需逐个添加。
  *
  *          计划模块（按实现顺序）：
  *            1. ADC 采集    —— PA2/PA3/PA5/PB1，DMA 循环
  *            2. 光电编码器  —— TIM2 Encoder 模式（PA0/PA1）
  *            3. PWM 输出    —— TIM3 CH1/CH2(CH3) @20kHz
  *            4. DS18B20     —— PA4 单总线
  *            5. AS5600      —— I2C1 磁编码器（PB6/PB7）
  *            6. 串口协议    —— USART1 115200
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

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  重定向 printf 到 USART1。
  * @param  ch 待发送字符
  * @retval 原字符
  * @note   逐字节阻塞发送，调试阶段够用。接上位机协议后改成
  *         环形缓冲 + DMA 发送，避免阻塞任务。
  */
int __io_putchar(int ch)
{
  uint8_t c = (uint8_t)ch;

  (void)HAL_UART_Transmit(&huart1, &c, 1U, 100U);
  return ch;
}

/**
  * @brief  初始化应用层。
  * @note   由 FreeRTOS 的 StartDefaultTask 调用，返回后该任务自行退出。
  *         后续各模块的初始化和任务创建都在这里展开。
  * @retval None
  */
void App_Init(void)
{
  /* ---------------------------------------------------------------------
   * 板级自检：把关键配置打一遍，确认烧录的是新固件
   * ------------------------------------------------------------------- */
  printf("\r\n");
  printf("========== motor_drive ==========\r\n");
  printf("[BOOT] STM32F103C8T6 @ %lu Hz\r\n", (unsigned long)HAL_RCC_GetHCLKFreq());
  printf("[BOOT] ADC  : PA2/PA3/PA5/PB1  (4ch + DMA)\r\n");
  printf("[BOOT] ENC  : TIM2 PA0/PA1     (encoder TI12)\r\n");
  printf("[BOOT] PWM  : TIM3 PA6/PA7/PB0 (20 kHz)\r\n");
  printf("[BOOT] TEMP : PA4              (DS18B20 1-Wire)\r\n");
  printf("[BOOT] I2C  : PB6/PB7          (AS5600)\r\n");
  printf("[BOOT] UART : PA9/PA10         (115200 8N1)\r\n");
  printf("=================================\r\n");
  printf("[APP ] init done, idle\r\n\r\n");

  /* ---------------------------------------------------------------------
   * 后续在这里挂各模块：
   *
   *   Adc_Init();          // 启动 DMA 循环转换
   *   Encoder_Init();      // HAL_TIM_Encoder_Start(&htim2, ...)
   *   Pwm_Init();          // HAL_TIM_PWM_Start(&htim3, ...)
   *   DS18B20_Init();
   *   AS5600_Init();
   *
   *   osThreadNew(App_Task, NULL, &attr);
   * ------------------------------------------------------------------- */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
