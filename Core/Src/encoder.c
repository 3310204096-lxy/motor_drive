/**
  ******************************************************************************
  * @file    encoder.c
  * @brief   This file provides code for the incremental Hall encoder, which is
  *          captured by TIM2 in encoder interface mode (TI1 and TI2).
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
#include "encoder.h"
#include "tim.h"

/* USER CODE BEGIN 0 */

/* TIM2 是 16 位计数器，满量程 65536。这里用 int16_t 做差分运算，
   计数回绕（65535 -> 0 或 0 -> 65535）会被自动处理成正确的有符号增量，
   前提是两次采样之间的增量绝对值小于 32768。                         */

#define RAD_PER_COUNT   (6.283185307f / (float)ENCODER_CPR)

/* 这几个变量跨上下文访问：Encoder_Update() 在 TIM4 中断里执行，
   Encoder_GetCount() 等在主循环里读取，因此必须加 volatile，
   防止编译器把中间值缓存在寄存器里。                                   */
static volatile int16_t  s_last_raw = 0;   /* 上一次读到的 16 位计数        */
static volatile int16_t  s_delta    = 0;   /* 最近一次采样周期内的增量      */
static volatile int32_t  s_total    = 0;   /* 上电以来的累计计数，支持多圈  */

/* USER CODE END 0 */

/**
  * @brief  初始化霍尔编码器并启动 TIM2 编码器接口。
  * @retval None
  */
void Encoder_Init(void)
{
  /* 清零计数器，保证上电时累计值从 0 开始 */
  __HAL_TIM_SET_COUNTER(&htim2, 0U);
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);

  s_last_raw = (int16_t)__HAL_TIM_GET_COUNTER(&htim2);
  s_delta    = 0;
  s_total    = 0;
}

/**
  * @brief  采样一次编码器，更新增量与累计值。
  * @note   应由固定的控制周期调用（当前 1 ms / 次）。调用间隔的稳定性
  *         直接决定后续速度计算的准确度。
  * @retval None
  */
void Encoder_Update(void)
{
  int16_t now;

  now        = (int16_t)__HAL_TIM_GET_COUNTER(&htim2);
  s_delta    = (int16_t)(now - s_last_raw);
  s_last_raw = now;
  s_total   += s_delta;
}

/**
  * @brief  读取累计计数。
  * @retval 上电以来的累计计数（四倍频计数，未除以减速比）
  */
int32_t Encoder_GetCount(void)
{
  return s_total;
}

/**
  * @brief  读取最近一个采样周期的增量。
  * @retval 增量计数，正负表示方向
  */
int16_t Encoder_GetDelta(void)
{
  return s_delta;
}

/**
  * @brief  读取累计角度。
  * @retval 电机轴（或输出轴，取决于 ENCODER_GEAR_RATIO）的累计角度，弧度
  * @note   该值不取模，可超过 2*PI，用于观察多圈累积与丢步现象。
  */
float Encoder_GetAngleRad(void)
{
  float rad = (float)s_total * RAD_PER_COUNT;

#if (ENCODER_GEAR_RATIO > 1)
  rad /= (float)ENCODER_GEAR_RATIO;
#endif
  return rad;
}

/**
  * @brief  清零累计值与增量。
  * @retval None
  */
void Encoder_Reset(void)
{
  __HAL_TIM_SET_COUNTER(&htim2, 0U);
  s_last_raw = 0;
  s_delta    = 0;
  s_total    = 0;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
