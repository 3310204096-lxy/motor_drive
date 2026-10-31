/**
  ******************************************************************************
  * @file    app_main.h
  * @brief   Application layer entry points, called from main().
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
#ifndef __APP_MAIN_H
#define __APP_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup APP
  * @{
  */

/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  初始化应用外设并创建业务任务。
  * @note   由 FreeRTOS 的 StartDefaultTask 调用，返回后该任务自行退出。
  */
void App_Init(void);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __APP_MAIN_H */
