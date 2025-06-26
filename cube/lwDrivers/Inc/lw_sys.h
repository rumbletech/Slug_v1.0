/*
 * lw_sys.h
 *
 *  Created on: Jun 8, 2025
 *      Author: rumbl
 */

#ifndef INC_LW_SYS_H_
#define INC_LW_SYS_H_

#include "stm32f1xx.h"
#include <stdint.h>


#define LW_SYS_NVIC_N_PRIO_BITS 4U

#define LW_SYS_NVIC_PRIO_GROUP_4 0x00003U /* 4 bits Preemption Priority , 0 Bits for SubPriority */
#define LW_SYS_NVIC_PRIO_GROUP_4_N_PREEMPT_PRIO_BITS 4U
#define LW_SYS_NVIC_PRIO_GROUP_4_N_SUB_PRIO_BITS 0U

extern void lw_Sys_IRQ_Enable( IRQn_Type IRQn );
extern void lw_Sys_IRQ_Disable( IRQn_Type IRQn );
extern void lw_Sys_IRQ_Set_Priority( IRQn_Type IRQn , uint32_t preemptPrio , uint32_t subPrio );
extern void lw_Sys_SysTick_Handler( void );
extern void lw_Sys_Disable_JTAG ( void );
extern void lw_Sys_ReMap_USART1( void );
extern void lw_Sys_IRQ_Set_Priority_Group ( uint32_t group );
extern uint64_t lw_Sys_Get_Ticks( void );
extern void lw_Sys_Delay( uint64_t timeMs );


#endif /* INC_LW_SYS_H_ */
