/**
  ******************************************************************************
  * @file    sensor.h
  * @brief   This file contains the unified data structure shared by all
  *          position/speed sensors (Hall encoder, AS5600 magnetic encoder
  *          and future fusion output).
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
#ifndef __SENSOR_H
#define __SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/** @addtogroup SENSOR
  * @{
  */

/** @defgroup SENSOR_Flags Sensor fault flags
  * @{
  */
#define SENSOR_FLAG_NONE          0x00U  /*!< 无故障                              */
#define SENSOR_FLAG_NO_MAGNET     0x01U  /*!< AS5600 未检测到磁铁 (MD = 0)        */
#define SENSOR_FLAG_WEAK_FIELD    0x02U  /*!< 磁场过弱 (ML = 1)                   */
#define SENSOR_FLAG_STRONG_FIELD  0x04U  /*!< 磁场过强 (MH = 1)                   */
#define SENSOR_FLAG_COMM_ERROR    0x08U  /*!< I2C 通信失败                        */
/**
  * @}
  */

/**
  * @brief  传感器统一输出结构
  * @note   所有传感器（霍尔 / 磁编码器 / 融合结果）都填充这个结构，
  *         上层控制算法不感知数据来源。新增传感器只需实现自己的采样函数。
  */
typedef struct
{
  float    Angle;      /*!< 绝对角度，弧度 [0, 2*PI)                        */
  float    Speed;      /*!< 角速度，rad/s，正负表示方向                     */
  uint32_t Tick;       /*!< 采样时刻，ms                                    */
  uint8_t  Valid;      /*!< 1 = 本次数据有效，0 = 无效（上层应丢弃）        */
  uint8_t  Flags;      /*!< 故障标志位，见 SENSOR_FLAG_xxx                  */
} Sensor_Data_t;

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_H */
