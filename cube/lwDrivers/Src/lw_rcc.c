/*
 * lw_rcc.c
 *
 *  Created on: May 13, 2025
 *      Author: rumbl
 */


#include "lw_rcc.h"


static uint32_t sysClk = 72000000ULL;
static uint32_t apb1Clk = 36000000ULL;
static uint32_t apb2Clk = 72000000ULL;


void lw_RCC_Init( void ){

	 // Enable HSE (High-Speed External) clock
	 RCC->CR |= RCC_CR_HSEON;// Turn on HSE
	 while (!(RCC->CR & RCC_CR_HSERDY));// Wait for HSE to be ready

	 // Configure Flash latency for 72 MHz (2 wait states for 48 MHz < SYSCLK <= 72 MHz)
	 FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2;

	 // Configure PLL
	 // PLLCLK = HSE * PLLMUL, where HSE = 8 MHz
	 // To get 72 MHz: PLLMUL = 9 (8 MHz * 9 = 72 MHz)
	 RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL);
	 RCC->CFGR |= (1 << RCC_CFGR_PLLSRC_Pos);// Select HSE as PLL source
	 RCC->CFGR |= RCC_CFGR_PLLMULL9;// PLL multiplication factor = 9

	 // Configure APB1 and APB2 prescalers
	 // APB1: 72 MHz / 2 = 36 MHz (max 36 MHz for APB1)
	 // APB2: 72 MHz / 1 = 72 MHz
	 RCC->CFGR &= ~(RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
	 RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;// APB1 = HCLK / 2 = 36 MHz
	 RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;// APB2 = HCLK / 1 = 72 MHz

	 // Configure USB clock
	 // USBCLK = PLLCLK / 1.5 = 72 MHz / 1.5 = 48 MHz
	 RCC->CFGR &= ~RCC_CFGR_USBPRE;
	 RCC->CFGR |= (0 << RCC_CFGR_USBPRE_Pos);// USB prescaler = PLL / 1.5

	 // Enable PLL
	 RCC->CR |= RCC_CR_PLLON;
	 while (!(RCC->CR & RCC_CR_PLLRDY));// Wait for PLL to lock

	 // Select PLL as system clock source
	 RCC->CFGR &= ~RCC_CFGR_SW;
	 RCC->CFGR |= RCC_CFGR_SW_PLL;// Set PLL as system clock
	 while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);// Wait for switch to PLL

     sysClk = 72000000ULL;
     apb1Clk = 36000000ULL;
     apb2Clk = 72000000ULL;
}


uint32_t lw_RCC_Get_SYSCLK(void){
	return sysClk;
}

uint32_t lw_RCC_Get_PCLK(void* perph){

	if ((USART_TypeDef*)perph == USART1 ){
		return apb2Clk;
	}
	else if ((USART_TypeDef*)perph == USART2 ){
		return apb1Clk;
	}
	else if ((USART_TypeDef*)perph == USART3 ){
		return apb1Clk;
	}
}

/**
 * @brief Enables the clock for AFIO.
 */
void lw_RCC_Enable_AFIO(void) {
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN_Msk;
}

/**
 * @brief Enables the clock for PWR.
 */
void lw_RCC_Enable_PWR(void) {
    RCC->APB1ENR |= RCC_APB1ENR_PWREN_Msk;
}


/**
 * @brief Enables the clock for GPIOA.
 */
void lw_RCC_Enable_GPIOA(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN_Msk;
}

/**
 * @brief Disables the clock for GPIOA.
 */
void lw_RCC_Disable_GPIOA(void) {
    RCC->APB2ENR &= ~RCC_APB2ENR_IOPAEN_Msk;
}

/**
 * @brief Enables the clock for GPIOB.
 */
void lw_RCC_Enable_GPIOB(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN_Msk;
}

/**
 * @brief Disables the clock for GPIOB.
 */
void lw_RCC_Disable_GPIOB(void) {
    RCC->APB2ENR &= ~RCC_APB2ENR_IOPBEN_Msk;
}

/**
 * @brief Enables the clock for SPI1.
 */
void lw_RCC_Enable_SPI1(void) {
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
}

/**
 * @brief Disables the clock for SPI1.
 */
void lw_RCC_Disable_SPI1(void) {
    RCC->APB2ENR &= ~RCC_APB2ENR_SPI1EN;
}

/**
 * @brief Enables the clock for SPI2.
 */
void lw_RCC_Enable_SPI2(void) {
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
}


/**
 * @brief Disables the clock for PWR.
 */
void lw_RCC_Disable_PWR(void) {
    RCC->APB1ENR &= ~RCC_APB1ENR_PWREN_Msk;
}

/**
 * @brief Disables the clock for SPI2.
 */
void lw_RCC_Disable_SPI2(void) {
    RCC->APB1ENR &= ~RCC_APB1ENR_SPI2EN;
}

/**
 * @brief Enables the clock for USART1.
 */
void lw_RCC_Enable_USART1(void) {
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
}

/**
 * @brief Disables the clock for USART1.
 */
void lw_RCC_Disable_USART1(void) {
    RCC->APB2ENR &= ~RCC_APB2ENR_USART1EN;
}

/**
 * @brief Enables the clock for USART2.
 */
void lw_RCC_Enable_USART2(void) {
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
}

/**
 * @brief Disables the clock for USART2.
 */
void lw_RCC_Disable_USART2(void) {
    RCC->APB1ENR &= ~RCC_APB1ENR_USART2EN;
}

/**
 * @brief Enables the clock for USART3.
 */
void lw_RCC_Enable_USART3(void) {
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
}

/**
 * @brief Disables the clock for USART3.
 */
void lw_RCC_Disable_USART3(void) {
    RCC->APB1ENR &= ~RCC_APB1ENR_USART3EN;
}
