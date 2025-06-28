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
#include <com_channel.h>
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "common.h"
#include "SDCD.h"
#include "rgb.h"
#include "lw_sys.h"
#include "lw_rcc.h"
#include "com_channel.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "diskt.h"
#include "fsmt.h"
#include <string.h>

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


static const uint32_t supported_baud_rates[] = {1200,2400,4800,9600,14400,19200,28800,38400,57600,115200,128000,230400,250000,460800,500000};



#define QUEUE_DISK_REQ_ITEM_SIZE (sizeof(struct diskt_request_s))
#define QUEUE_DISK_REQ_LENGTH 6U

#define QUEUE_DISK_RESP_ITEM_SIZE (sizeof(struct diskt_response_s))
#define QUEUE_DISK_RESP_LENGTH 6U

#define TASK_DISKT_STACK_SIZE 512U
#define TASK_DISKT_PRIO 2U

static uint8_t queue_disk_req_buff[QUEUE_DISK_REQ_ITEM_SIZE*QUEUE_DISK_REQ_LENGTH];
static QueueHandle_t queue_disk_req;
static StaticQueue_t queue_disk_req_static;

static uint8_t queue_disk_resp_buff[QUEUE_DISK_RESP_LENGTH*QUEUE_DISK_RESP_ITEM_SIZE];
static QueueHandle_t queue_disk_resp;
static StaticQueue_t queue_disk_resp_static;

static TaskHandle_t taskHandle_DISKT = NULL;
static StackType_t taskStack_DISKT [ TASK_DISKT_STACK_SIZE ];
static StaticTask_t taskBuffer_DISKT;

#define TASK_FSMT_STACK_SIZE 256U
#define TASK_FSMT_PRIO 2U
static TaskHandle_t taskHandle_FSMT = NULL;
static StackType_t taskStack_FSMT [ TASK_FSMT_STACK_SIZE ];
static StaticTask_t taskBuffer_FSMT;






//static bool assertBaud( uint32_t baudRate ){
//	bool ret = false;
//	for ( uint32_t i = 0 ; i < sizeof(supported_baud_rates)/sizeof(uint32_t) ;i++ ){
//		if ( baudRate == supported_baud_rates[i]){
//			ret = true;
//			break;
//		}
//	}
//	return ret;
//}

//static bool assertConfiguration( lw_uart_data_t cfg ){
//
//	if ( !assertBaud(cfg.baudRate)  ){
//		return false;
//	}
//
//	if ( cfg.parity >= e_uch_parity_MAX_ ){
//		return false;
//	}
//
//	if ( cfg.nStopBits >= e_uch_nStopbits_MAX_ ){
//		return false;
//	}
//
//	if ( cfg.nDataBits >= e_uch_nDatabits_MAX_ ){
//		return false;
//	}
//
//	return true;
//}

extern volatile uint32_t uart_count;
#define UCH_CONFIG_MAX_TOKEN_SIZE 64u


/* Init json parser */
//	jsmn_init(&uch.json.parser);







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

void vApplicationTickHook ( void ){
	lw_Sys_SysTick_Handler();
}
/* Executed as First Step During Startup ( Before Main ) */
extern void SystemInit ( void ){
	/* Init System Clock and Bus Clocks */
	lw_RCC_Init();
	/* Set NVIC Priority Group to 4 Preemption Prio Bits and 0 SubPriority Bits */
	lw_Sys_IRQ_Set_Priority_Group(LW_SYS_NVIC_PRIO_GROUP_4);

	/* Set Interrupt Priorities */
	lw_Sys_IRQ_Set_Priority(SysTick_IRQn, 15U , 0U);
	lw_Sys_IRQ_Set_Priority(PendSV_IRQn,  15u , 0U);
	lw_Sys_IRQ_Set_Priority(SVCall_IRQn,  15u , 0U);

	/* Enable AFIO and PWR Domains */
	lw_RCC_Enable_AFIO();
	lw_RCC_Enable_PWR();

	/* AIFO and RE-MAP */
	lw_Sys_Disable_JTAG(); /* Remap JTAG to SWD */
	lw_Sys_ReMap_USART1(); /* Remap USART1 */

	return;
}


void vTask_DISKT ( void *pvParameters ){

	DISKT_Init ( queue_disk_req , queue_disk_resp );

	for(;;){
		DISKT_Process();
	}

}

void vTask_FSMT ( void *pvParameters ){

	fsm_init();

	for(;;){
		fsm();
		vTaskDelay(FSM_CYCLE_DURATION_MS);
	}

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

  __disable_irq();

  RGB_Init(); /* Initializes RGB Indicator Module */
  SDCD_Init(); /* Initializes SDC SPI Module */
  Com_Channel_Init(); /* Initializes Com Channels , as well as debug channel if enabled */
  MX_FATFS_Init(); /* Link FatFS Driver */


  /* Create Queues */

  queue_disk_req = xQueueCreateStatic( QUEUE_DISK_REQ_LENGTH,QUEUE_DISK_REQ_ITEM_SIZE,queue_disk_req_buff,&queue_disk_req_static );
  queue_disk_resp = xQueueCreateStatic( QUEUE_DISK_RESP_LENGTH,QUEUE_DISK_RESP_ITEM_SIZE,queue_disk_resp_buff,&queue_disk_resp_static );


  /* Create Tasks */
  taskHandle_DISKT = xTaskCreateStatic(
		  	  	  	   vTask_DISKT,           /* Function that implements the task. */
                       "DISKT",               /* Text name for the task. */
                       TASK_DISKT_STACK_SIZE, /* Number of indexes in the xStack array. */
                       ( void * ) 1,          /* Parameter passed into the task. */
                       TASK_DISKT_PRIO,       /* Priority at which the task is created. */
					   taskStack_DISKT,       /* Array to use as the task's stack. */
                       &taskBuffer_DISKT );   /* Variable to hold the task's data structure. */

  taskHandle_FSMT = xTaskCreateStatic(
		  	  	  	   vTask_FSMT,           /* Function that implements the task. */
                       "FSMT",               /* Text name for the task. */
                       TASK_FSMT_STACK_SIZE, /* Number of indexes in the xStack array. */
                       ( void * ) 1,         /* Parameter passed into the task. */
                       TASK_FSMT_PRIO,       /* Priority at which the task is created. */
					   taskStack_FSMT,       /* Array to use as the task's stack. */
                       &taskBuffer_FSMT );   /* Variable to hold the task's data structure. */

  /* Start the scheduler */
  vTaskStartScheduler();

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
  Common_Printf("FUCK MY ASS \r\n");
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
