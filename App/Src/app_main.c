/**
  ******************************************************************************
  * @file    app_main.c
  * @brief   应用层入口（空骨架）。
  *
  *          当前状态：不含任何模块，仅保留 FreeRTOS 启动后的挂载点。
  *
  *          开发方式：每个功能模块在独立分支上开发并单独验证，
  *                    验证通过后合并回 develop，最后再统一整理架构。
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
#include "cmsis_os.h"

#include "main.h"
#include "app_main.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  初始化应用层。
  * @note   由 FreeRTOS 的 StartDefaultTask 调用，返回后该任务自行退出。
  * @retval None
  */
void App_Init(void)
{
  /* 各模块初始化在此展开 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
