/**
  ******************************************************************************
  * @file    app_tasks.h
  * @brief   FreeRTOS task entry points for the application layer.
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
#ifndef __APP_TASKS_H
#define __APP_TASKS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup APP
  * @{
  */

/* Exported functions prototypes ---------------------------------------------*/
void App_SensorTask(void *argument);
void App_ReportTask(void *argument);

/**
  * @brief  打印自检表头（在任务启动前调用一次）。
  */
void App_PrintHeader(void);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __APP_TASKS_H */
