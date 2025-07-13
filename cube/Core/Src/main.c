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

#include "main.h"
#include "fatfs.h"
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

	  __disable_irq();

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

  SystemInit();  /* Init System Clock and NVIC  */
  RGB_Init(); /* Initializes RGB Indicator Module */
  SDCD_Init(); /* Initializes SDC SPI Module */
  Com_Channel_Init(); /* Initializes Com Channels , as well as debug channel if enabled */
  MX_FATFS_Init(); /* Link FatFS Driver */



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
  __disable_irq();
  Common_Printf("Error_Handler\r\n");
  while (1)
  {
  }
}

