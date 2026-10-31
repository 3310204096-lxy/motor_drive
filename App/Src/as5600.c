/**
  ******************************************************************************
  * @file    as5600.c
  * @brief   This file provides code for the AS5600 12-bit magnetic rotary
  *          position sensor, read over I2C1.
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
#include "as5600.h"
#include "i2c.h"

/* USER CODE BEGIN 0 */

/* AS5600 输出绝对角度，不存在累积误差，但每次读数都需要一次完整的 I2C 事务。
   100 kHz 下读两个字节约 540 us，400 kHz 下约 135 us——这是采样率的主要瓶颈，
   也是与霍尔（纳秒级读取）形成互补的原因。                            */

static uint8_t s_last_status = 0U;   /* 缓存最近一次 STATUS 寄存器值          */

/* USER CODE END 0 */

/**
  * @brief  初始化 AS5600，并确认 I2C 通信正常。
  * @retval 1 = 通信正常，0 = 无响应
  * @note   AS5600 默认配置即可输出角度，这里只做一次连通性探测。
  */
uint8_t AS5600_Init(void)
{
  uint8_t status = 0U;

  if (AS5600_ReadStatus(&status) == 0U)
  {
    return 0U;
  }
  s_last_status = status;
  return 1U;
}

/**
  * @brief  读取 STATUS 寄存器。
  * @param  status 输出，寄存器原始值，见 AS5600_STATUS_xxx
  * @retval 1 = 读取成功，0 = 失败
  */
uint8_t AS5600_ReadStatus(uint8_t *status)
{
  if (status == NULL)
  {
    return 0U;
  }

  if (HAL_I2C_Mem_Read(&hi2c1, AS5600_I2C_ADDR, AS5600_REG_STATUS,
                       I2C_MEMADD_SIZE_8BIT, status, 1U,
                       AS5600_I2C_TIMEOUT) != HAL_OK)
  {
    return 0U;
  }
  return 1U;
}

/**
  * @brief  读取 12 位原始角度。
  * @param  raw 输出，范围 0 ~ 4095
  * @retval 1 = 读取成功，0 = 失败
  */
uint8_t AS5600_ReadRawAngle(uint16_t *raw)
{
  uint8_t buf[2];

  if (raw == NULL)
  {
    return 0U;
  }

  if (HAL_I2C_Mem_Read(&hi2c1, AS5600_I2C_ADDR, AS5600_REG_RAW_ANGLE_H,
                       I2C_MEMADD_SIZE_8BIT, buf, 2U,
                       AS5600_I2C_TIMEOUT) != HAL_OK)
  {
    return 0U;
  }

  /* 高字节只有低 4 位有效 */
  *raw = (uint16_t)(((uint16_t)(buf[0] & 0x0FU) << 8) | (uint16_t)buf[1]);
  return 1U;
}

/**
  * @brief  把 12 位原始值换算为弧度。
  * @param  raw 原始角度 0 ~ 4095
  * @retval 角度，弧度 [0, 2*PI)
  */
float AS5600_RawToRad(uint16_t raw)
{
  return (float)raw * AS5600_ANGLE_STEP;
}

/**
  * @brief  采样一次磁编码器，填充统一的传感器结构。
  * @param  out 输出结构，调用者提供
  * @param  raw 可选输出，原始 12 位角度；传 NULL 表示不关心
  * @retval 1 = 数据有效，0 = 本次读取失败
  * @note   读取失败时保留上一次的角度值并置 Valid = 0，而不是清零。
  *         清零会被上层 PID 当作一次巨大的位置跳变。
  */
uint8_t AS5600_Sample(Sensor_Data_t *out, uint16_t *raw)
{
  uint16_t raw_val = 0U;
  uint8_t  status = 0U;

  if (out == NULL)
  {
    return 0U;
  }

  out->Flags = SENSOR_FLAG_NONE;

  if (AS5600_ReadRawAngle(&raw_val) == 0U)
  {
    out->Valid  = 0U;
    out->Flags |= SENSOR_FLAG_COMM_ERROR;
    return 0U;
  }

  if (AS5600_ReadStatus(&status) != 0U)
  {
    s_last_status = status;
  }

  if ((s_last_status & AS5600_STATUS_MD) == 0U)
  {
    out->Flags |= SENSOR_FLAG_NO_MAGNET;
  }
  if ((s_last_status & AS5600_STATUS_ML) != 0U)
  {
    out->Flags |= SENSOR_FLAG_WEAK_FIELD;
  }
  if ((s_last_status & AS5600_STATUS_MH) != 0U)
  {
    out->Flags |= SENSOR_FLAG_STRONG_FIELD;
  }

  if (raw != NULL)
  {
    *raw = raw_val;
  }

  out->Angle = AS5600_RawToRad(raw_val);
  out->Valid = 1U;

  return 1U;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
