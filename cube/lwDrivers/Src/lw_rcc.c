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
