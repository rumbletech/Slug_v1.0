/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "common.h"
#include "SDCD.h"
#include "rgb.h"
#include "file_sys.h"
#include "uart_channel.h"

#include "lw_sys.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

extern volatile uint32_t uart_count;

static void TEST_SDCARD( void ){
	static uint32_t index = 0u;
	static uint32_t failCount = 0u;

	BYTE sec[512u];
	failCount += (uint32_t)SD_disk_read(0, &sec[0u], index+8192 , 1);
	Common_Printf("Sector %d = \r\n" , index);
	Common_Printf("Total Fail %d = \r\n" , failCount);

	index++;
}

static void TEST_RGB ( void ){

      static uint32_t color = 0u;
	  RGB_Write(color++);
}

/* Executed as First Step During Startup ( Before Main ) */
extern void SystemInit ( void ){
	/* Init NVIC , System Clock and SysTick */
	lw_Sys_Init();
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  (void)(TEST_SDCARD);
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  SystemInit();
  //HAL_MspInit();
  /* USER CODE BEGIN Init */

  //SystemClock_Config();
  /* USER CODE END Init */

  /* Configure the system clock */
 // SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
//  MX_GPIO_Init();
//  MX_SPI1_Init();
//  MX_USART1_UART_Init();
//  MX_USART2_UART_Init();
//  huart3.hwctx = _DEBUG_PRINTF_PORT_;
//  huart3.data.baudRate = _DEBUG_PRINTF_BAUD_;
//  huart3.data.dataBits = _DEBUG_PRINTF_WLEN_;
//  huart3.data.stopBits = _DEBUG_PRINTF_STOPB_;
//  huart3.data.parity = LW_UART_PARITY_NONE;
//  lw_UART_Init(&huart3);
  MX_FATFS_Init();
//  MX_RTC_Init();
//  MX_USB_PCD_Init();
  /* USER CODE BEGIN 2 */

  RGB_Init();
  RGB_Write(COLOR_CAYAN);
  SDCD_Init();
  uch_Init();

  _fsys_err res = Fsys_Init();
  Common_Printf("Hello World \r\n");
  if(res != FSYS_SUCCESS) {
	  Common_Printf("Mount Failed %d\r\n", res);
	  while(1);
  }
  /* Load Config File into Memory */
  Fsys_LoadConfig();
  /* Enumerate Channels from SDC */
  uch_EnumerateChannels();

  uch_ApplyConfiguration(CHANNEL_1);
  uch_ApplyConfiguration(CHANNEL_2);
  uch_ApplyConfiguration(CHANNEL_3);

  Common_Printf("Enumeration Successful Debug Port Open\r\n");

  //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
	//  TEST_SDCARD();
	  Common_Printf("yo %c \r\n" , uart_count);
	  TEST_RGB();
	  lw_Sys_Delay(1000u);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}


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

#ifdef  USE_FULL_ASSERT
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
