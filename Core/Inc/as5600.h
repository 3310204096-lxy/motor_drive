/**
  ******************************************************************************
  * @file    as5600.h
  * @brief   This file contains all the function prototypes for the AS5600
  *          magnetic rotary position sensor connected to I2C1.
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
#ifndef __AS5600_H
#define __AS5600_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "sensor.h"

/** @addtogroup AS5600
  * @{
  */

/** @defgroup AS5600_Registers Register map
  * @{
  */
#define AS5600_I2C_ADDR           (0x36U << 1U) /*!< 7 位地址 0x36 左移，HAL 需要 8 位形式 */
#define AS5600_REG_STATUS         0x0BU         /*!< 磁铁检测状态                          */
#define AS5600_REG_RAW_ANGLE_H    0x0CU         /*!< 原始角度高 4 位                       */
#define AS5600_REG_RAW_ANGLE_L    0x0DU         /*!< 原始角度低 8 位                       */
/**
  * @}
  */

/** @defgroup AS5600_StatusBits STATUS register bits
  * @{
  */
#define AS5600_STATUS_MD          0x20U  /*!< 1 = 检测到磁铁                        */
#define AS5600_STATUS_ML          0x10U  /*!< 1 = 磁场过弱                          */
#define AS5600_STATUS_MH          0x08U  /*!< 1 = 磁场过强                          */
/**
  * @}
  */

/** @defgroup AS5600_Constants Resolution
  * @{
  */
#define AS5600_RESOLUTION         4096.0f  /*!< 每圈 12 位，4096 个计数             */
#define AS5600_ANGLE_STEP         (6.283185307f / AS5600_RESOLUTION)
/**
  * @}
  */

/** @defgroup AS5600_Timeout I2C timeout
  * @{
  */
#define AS5600_I2C_TIMEOUT        10U   /*!< 单次读写的超时，ms                     */
/**
  * @}
  */

/* Exported functions prototypes ---------------------------------------------*/
uint8_t AS5600_Init(void);
uint8_t AS5600_ReadRawAngle(uint16_t *raw);
uint8_t AS5600_ReadStatus(uint8_t *status);
uint8_t AS5600_Sample(Sensor_Data_t *out, uint16_t *raw);
float   AS5600_RawToRad(uint16_t raw);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __AS5600_H */
