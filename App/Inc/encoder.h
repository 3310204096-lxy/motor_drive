/**
  ******************************************************************************
  * @file    encoder.h
  * @brief   This file contains all the function prototypes for the incremental
  *          Hall encoder connected to TIM2 in encoder interface mode.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ENCODER_H
#define __ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "sensor.h"

/** @addtogroup ENCODER
  * @{
  */

/** @defgroup ENCODER_Configuration Encoder configuration
  * @{
  */

/* 霍尔编码器线数（每相每转脉冲数）。必须按实际电机型号修改，
   常见 11 线 / 13 线。该值决定所有速度与角度换算的基准。            */
#define ENCODER_PPR           13U

/* 四倍频后的每转计数（TI12 模式对 A/B 相双边沿都计数）              */
#define ENCODER_CPR           (ENCODER_PPR * 4U)

/* 电机减速比（输出轴 : 电机轴）。若直接驱动电机轴，填 1。            */
#define ENCODER_GEAR_RATIO    1U

/* 控制周期，ms。与 TIM4 定时中断频率保持一致。                      */
#define CONTROL_PERIOD_MS     1U

/**
  * @}
  */

/* Exported functions prototypes ---------------------------------------------*/
void    Encoder_Init(void);
void    Encoder_Update(void);
int32_t Encoder_GetCount(void);
int16_t Encoder_GetDelta(void);
float   Encoder_GetAngleRad(void);
void    Encoder_Reset(void);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __ENCODER_H */
