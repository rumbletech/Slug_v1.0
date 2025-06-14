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

void lw_Sys_IRQ_Set_Priority_Group ( uint32_t group ){
	NVIC_SetPriorityGrouping(group);
}

extern void lw_Sys_Disable_JTAG ( void ){
	/* Remap JTAG to SWD only */
	AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE_Msk;
}

void lw_Sys_SysTick_Handler( void ){
	ticks++;
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

