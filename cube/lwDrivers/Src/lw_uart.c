/*
 * lw_uart.c
 *
 *  Created on: May 30, 2025
 *      Author: rumbl
 */


#include "lw_uart.h"

#define UART_DIV_SAMPLING16(_PCLK_, _BAUD_)            (((_PCLK_)*25U)/(4U*(_BAUD_)))
#define UART_DIVMANT_SAMPLING16(_PCLK_, _BAUD_)        (UART_DIV_SAMPLING16((_PCLK_), (_BAUD_))/100U)
#define UART_DIVFRAQ_SAMPLING16(_PCLK_, _BAUD_)        ((((UART_DIV_SAMPLING16((_PCLK_), (_BAUD_)) - (UART_DIVMANT_SAMPLING16((_PCLK_), (_BAUD_)) * 100U)) * 16U)\
                                                         + 50U) / 100U)
/* UART BRR = mantissa + overflow + fraction
            = (UART DIVMANT << 4) + (UART DIVFRAQ & 0xF0) + (UART DIVFRAQ & 0x0FU) */
#define UART_BRR_SAMPLING16(_PCLK_, _BAUD_)            (((UART_DIVMANT_SAMPLING16((_PCLK_), (_BAUD_)) << 4U) + \
                                                         (UART_DIVFRAQ_SAMPLING16((_PCLK_), (_BAUD_)) & 0xF0U)) + \
                                                        (UART_DIVFRAQ_SAMPLING16((_PCLK_), (_BAUD_)) & 0x0FU))


/* Initialize UART Device */
void lw_UART_Init(lw_uart* inst) {
    // Validate input
    if (inst == NULL || inst->hwctx == NULL) {
        return;
    }

    // Reset USART control registers
    inst->hwctx->CR1 = 0;
    inst->hwctx->CR2 = 0;
    inst->hwctx->CR3 = 0;

    // Set baud rate
    inst->hwctx->BRR = UART_BRR_SAMPLING16(lw_RCC_Get_PCLK(inst->hwctx),inst->data.baudRate);

    // Configure data bits
    if (inst->data.dataBits == LW_UART_NUM_DATA_BITS_9) {
        inst->hwctx->CR1 |= USART_CR1_M; // 9-bit data
    } else {
        inst->hwctx->CR1 &= ~USART_CR1_M; // 8-bit data
    }

    // Configure stop bits
    if (inst->data.stopBits == LW_UART_NUM_STOP_BITS_2) {
        inst->hwctx->CR2 |= USART_CR2_STOP_1; // 2 stop bits
    } else {
        inst->hwctx->CR2 &= ~USART_CR2_STOP; // 1 stop bit
    }

    // Configure parity
    if (inst->data.parity != LW_UART_PARITY_NONE) {
        inst->hwctx->CR1 |= USART_CR1_PCE; // Enable parity
        if (inst->data.parity == LW_UART_PARITY_ODD) {
            inst->hwctx->CR1 |= USART_CR1_PS; // Odd parity
        } else {
            inst->hwctx->CR1 &= ~USART_CR1_PS; // Even parity
        }
    } else {
        inst->hwctx->CR1 &= ~USART_CR1_PCE; // No parity
    }

    // Enable transmitter, receiver, and USART
    inst->hwctx->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

// Receive Data from Hardware
uint8_t lw_UART_Receieve(lw_uart* inst ) {
    return inst->hwctx->DR;
}

// Transmit data over UART
void lw_UART_Transmit(lw_uart* inst, uint8_t* data, uint32_t length) {
    if (inst == NULL || inst->hwctx == NULL || data == NULL) {
        return;
    }
    for (uint32_t i = 0; i < length; i++) {
        while (!(inst->hwctx->SR & USART_SR_TXE)); // Wait for TX buffer empty
        inst->hwctx->DR = data[i]; // Send character
        while (!(inst->hwctx->SR & USART_SR_TC)); // Wait for transmission complete
    }
}

// Enable IRQ
void lw_UART_EnableIRQ(lw_uart* inst , lw_uart_irq_t irq){
	if ( inst == NULL ){
		return;
	}

	switch( irq ){
	case LW_UART_IRQ_RXNE:
		inst->hwctx->CR1 |= USART_CR1_RXNEIE_Msk;
		break;
	default:
		break;
	}
}

void lw_UART_DisableIRQ(lw_uart* inst , lw_uart_irq_t irq){
	if ( inst == NULL ){
		return;
	}

	switch( irq ){
	case LW_UART_IRQ_RXNE:
		inst->hwctx->CR1 &= ~USART_CR1_RXNEIE_Msk;
		break;
	default:
		break;
	}
}

