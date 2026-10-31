/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include "sensor.h"
#include "encoder.h"
#include "as5600.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* 由 TIM4 更新中断维护的毫秒节拍 */
volatile uint32_t g_tick_ms = 0U;

/* P1 阶段的采样与上报节拍 */
static uint32_t s_last_mag   = 0U;
static uint32_t s_last_print = 0U;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

static const char *P1_FlagStr(uint8_t flags);
static void        P1_PrintHeader(void);
static void        P1_PrintStatus(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* ---------- P1：双传感器自检阶段的时间参数（ms） ---------- */
#define P1_MAG_SAMPLE_MS     10U    /* 磁编码器采样间隔                  */
#define P1_PRINT_MS          200U   /* 串口上报间隔                      */

/* 最近一次磁编码器采样结果，供打印使用 */
static Sensor_Data_t s_mag;
static uint16_t      s_mag_raw = 0U;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  /* 1 kHz 控制周期时基 */
  MX_TIM4_Init();
  HAL_TIM_Base_Start_IT(&htim4);

  /* 启动霍尔编码器（TIM2 编码器接口） */
  Encoder_Init();

  /* P1 自检：确认两路传感器都能出数 */
  P1_PrintHeader();

  if (AS5600_Init() == 0U)
  {
    printf("[P1] AS5600 no response, check I2C wiring / pull-ups\r\n");
  }
  else
  {
    printf("[P1] AS5600 online\r\n");
  }
  printf("[P1] Turn the motor shaft by hand at a steady speed.\r\n\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if ((g_tick_ms - s_last_mag) >= P1_MAG_SAMPLE_MS)
    {
      s_last_mag = g_tick_ms;
      (void)AS5600_Sample(&s_mag, &s_mag_raw);
    }

    if ((g_tick_ms - s_last_print) >= P1_PRINT_MS)
    {
      s_last_print = g_tick_ms;
      P1_PrintStatus();
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
  * @brief  定时器更新中断回调。
  * @param  htim 触发的定时器句柄
  * @retval None
  * @note   TIM4 每 1 ms 进入一次：推进系统节拍，同时采样霍尔增量。
  *         磁编码器走 I2C，耗时在百微秒量级，不能放在这里。
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM4)
  {
    g_tick_ms++;
    Encoder_Update();
  }
}

/**
  * @brief  把故障标志转换为可读字符串。
  */
static const char *P1_FlagStr(uint8_t flags)
{
  if (flags == SENSOR_FLAG_NONE)        { return "OK";     }
  if ((flags & SENSOR_FLAG_COMM_ERROR) != 0U)   { return "COMM";   }
  if ((flags & SENSOR_FLAG_NO_MAGNET) != 0U)    { return "NOMAG";  }
  if ((flags & SENSOR_FLAG_WEAK_FIELD) != 0U)   { return "WEAK";   }
  if ((flags & SENSOR_FLAG_STRONG_FIELD) != 0U) { return "STRONG"; }
  return "UNKNOWN";
}

/**
  * @brief  打印自检表头，说明当前使用的换算基准。
  */
static void P1_PrintHeader(void)
{
  printf("\r\n==================================================\r\n");
  printf(" Motor Drive - P1 Sensor Checkout\r\n");
  printf(" Build : %s %s\r\n", __DATE__, __TIME__);
  printf(" Hall  : TIM2 encoder, %u PPR x 4 = %u CPR, gear 1:%u\r\n",
         (unsigned int)ENCODER_PPR,
         (unsigned int)ENCODER_CPR,
         (unsigned int)ENCODER_GEAR_RATIO);
  printf(" Mag   : AS5600 12-bit, I2C1 @ %lu Hz, addr 0x36\r\n",
         (unsigned long)hi2c1.Init.ClockSpeed);
  printf("==================================================\r\n\r\n");
}

/**
  * @brief  打印一次两路传感器的当前读数。
  * @note   刻意不使用 %f——newlib-nano 默认不链接浮点格式化，
  *         这里手工拆整数与小数部分，兼顾体积与可读性。
  */
static void P1_PrintStatus(void)
{
  int32_t  hall     = Encoder_GetCount();
  float    hall_deg = (float)hall * 360.0f / (float)ENCODER_CPR;
  int32_t  hd10     = (int32_t)(hall_deg * 10.0f);
  int32_t  hd_i     = hd10 / 10;
  int32_t  hd_f     = hd10 % 10;

  float    mag_deg  = s_mag.Angle * 57.295780f;
  int32_t  md10     = (int32_t)(mag_deg * 10.0f);
  int32_t  md_i     = md10 / 10;
  int32_t  md_f     = md10 % 10;

  /* 负数取模为负，单独取绝对值，避免打印成 "87.-3" */
  if (hd_f < 0) { hd_f = -hd_f; }
  if (md_f < 0) { md_f = -md_f; }

  printf("t=%6lu  H=%8ld (%6ld.%01ld deg)  M=%4u (%5ld.%01ld deg)  %s%s\r\n",
         (unsigned long)g_tick_ms,
         (long)hall,
         (long)hd_i, (long)hd_f,
         (unsigned int)s_mag_raw,
         (long)md_i, (long)md_f,
         P1_FlagStr(s_mag.Flags),
         (s_mag.Valid != 0U) ? "" : " INVALID");
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
