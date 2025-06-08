/*
 * lw_rcc.h
 *
 *  Created on: May 13, 2025
 *      Author: rumbl
 */

#ifndef INC_LW_RCC_H_
#define INC_LW_RCC_H_

#include <stdint.h>
#include "common.h"
#include "stm32f1xx.h"

//Initialize System Clock
void lw_RCC_Init( void );

// Get System Clock
uint32_t lw_RCC_Get_SYSCLK(void);
// Get Peripheral Clock
uint32_t lw_RCC_Get_PCLK(void* perph);

//Clock enable function for PWR
void lw_RCC_Enable_PWR(void);

//Clock enable function for GPIO AFIO
void lw_RCC_Enable_AFIO(void);

// Clock enable functions for GPIO peripherals
void lw_RCC_Enable_GPIOA(void);
void lw_RCC_Enable_GPIOB(void);

// Clock enable functions for SPI peripherals
void lw_RCC_Enable_SPI1(void);
void lw_RCC_Enable_SPI2(void);

// Clock enable functions for UART peripherals
void lw_RCC_Enable_USART1(void);
void lw_RCC_Enable_USART2(void);
void lw_RCC_Enable_USART3(void);

//Clock disable function for PWR
void lw_RCC_Disable_PWR(void);

// Clock disable functions for GPIO peripherals
void lw_RCC_Disable_GPIOA(void);
void lw_RCC_Disable_GPIOB(void);

// Clock disable functions for SPI peripherals
void lw_RCC_Disable_SPI1(void);
void lw_RCC_Disable_SPI2(void);

// Clock disable functions for UART peripherals
void lw_RCC_Disable_USART1(void);
void lw_RCC_Disable_USART2(void);
void lw_RCC_Disable_USART3(void);

#endif /* INC_LW_RCC_H_ */
