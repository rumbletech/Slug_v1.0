/*
 * lw_sys.c
 *
 *  Created on: Jun 8, 2025
 *      Author: rumbl
 */

#include "lw_sys.h"
#include "core_cm3.h"
#include "lw_rcc.h"

#define LW_SYS_ticksICK_FREQ 1000U

#define LW_SYS_NVIC_N_PRIO_BITS 4U
#define LW_SYS_NVIC_PRIO_GROUP_4 0x00003U /* 4 bits Preemption Priority , 0 Bits for SubPriority */

#define LW_SYS_NVIC_PRIO_GROUP_4_N_PREEMPT_PRIO_BITS 4U
#define LW_SYS_NVIC_PRIO_GROUP_4_N_SUB_PRIO_BITS 0U

static volatile uint64_t ticks = 0u;

void lw_Sys_IRQ_Enable( IRQn_Type IRQn ){
	NVIC_EnableIRQ(IRQn);
}

void lw_Sys_IRQ_Disable( IRQn_Type IRQn ){
	NVIC_DisableIRQ(IRQn);
}

void lw_Sys_IRQ_Set_Priority( IRQn_Type IRQn , uint32_t preemptPrio , uint32_t subPrio ){
	uint32_t prioritygroup = NVIC_GetPriorityGrouping();
	NVIC_SetPriority(IRQn, NVIC_EncodePriority(prioritygroup, preemptPrio, subPrio));
}

static void disableJTAG ( void ){
	/* Remap JTAG to SWD only */
	AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE_Msk;

}

uint64_t lw_Sys_Get_Ticks( void ){
	return ticks;
}

void lw_Sys_Delay( uint64_t delay ){

	uint64_t t = ticks;
	uint64_t dt = 0ul;

	do{
		dt = ticks - t;
	}
	while( dt < delay );
}

void lw_Sys_Init( void ){
	/* Initialize ticksem Clocks */
	lw_RCC_Init();
	/* All bits for pre-emption priority */
	NVIC_SetPriorityGrouping(LW_SYS_NVIC_PRIO_GROUP_4);
	/* Configure 1 ms ticksick */
	SysTick_Config(lw_RCC_Get_SYSCLK()/LW_SYS_ticksICK_FREQ);
	/* Set ticksick Prio */
	lw_Sys_IRQ_Set_Priority(SysTick_IRQn, 13U , 0U);
	lw_Sys_IRQ_Set_Priority(PendSV_IRQn,  15u , 0U);
	lw_Sys_IRQ_Set_Priority(SVCall_IRQn,  14u , 0U);


	lw_Sys_IRQ_Disable(SysTick_IRQn);
	lw_Sys_IRQ_Disable(PendSV_IRQn);
	lw_Sys_IRQ_Disable(SVCall_IRQn);

	lw_RCC_Enable_AFIO();
	lw_RCC_Enable_PWR();

	disableJTAG();

}
