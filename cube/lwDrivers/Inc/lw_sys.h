/*
 * lw_sys.h
 *
 *  Created on: Jun 8, 2025
 *      Author: rumbl
 */

#ifndef INC_LW_SYS_H_
#define INC_LW_SYS_H_

#include "stm32f1xx.h"
#include "stdint.h"

void lw_Sys_IRQ_Enable( IRQn_Type IRQn );
void lw_Sys_IRQ_Disable( IRQn_Type IRQn );
void lw_Sys_IRQ_Set_Priority( IRQn_Type IRQn , uint32_t preemptPrio , uint32_t subPrio );
void lw_Sys_Init( void );

uint64_t lw_Sys_Get_Ticks( void );
void lw_Sys_Delay( uint64_t timeMs );

#endif /* INC_LW_SYS_H_ */
