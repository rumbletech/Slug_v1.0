/*
 * lw_uart.h
 *
 *  Created on: May 30, 2025
 *      Author: rumbl
 */

#ifndef INC_LW_UART_H_
#define INC_LW_UART_H_

#include <stdint.h>
#include <stm32f1xx.h>

typedef enum {
	LW_UART_PARITY_NONE,   /* No Parity */
	LW_UART_PARITY_EVEN,  /* Even Parity */
	LW_UART_PARITY_ODD    /* Odd Parity */
} lw_uart_parity_t;

typedef enum {
	LW_UART_NUM_STOP_BITS_1, /* 1 Stop Bits */
	LW_UART_NUM_STOP_BITS_2  /* 2 Stop Bits */
} lw_uart_num_stop_bits_t;

typedef enum {
	LW_UART_NUM_DATA_BITS_8, /* 8 Data Bits */
	LW_UART_NUM_DATA_BITS_9  /* 9 Data Bits */
} lw_uart_num_data_bits_t;

typedef struct {
	uint32_t baudRate;
	lw_uart_parity_t parity;
	lw_uart_num_stop_bits_t stopBits;
	lw_uart_num_data_bits_t dataBits;
} lw_uart_data_t;

typedef enum {
	LW_UART_IRQ_RXNE,
	LW_UART_IRQ_PERR,
	LW_UART_IRQ_ERR,
} lw_uart_irq_t;

typedef struct {
	USART_TypeDef* hwctx;
	lw_uart_data_t data;
} lw_uart;

void lw_UART_Init(lw_uart* inst);
void lw_UART_Transmit(lw_uart* inst,uint8_t* data , uint32_t length);
void lw_UART_EnableIRQ(lw_uart* inst , lw_uart_irq_t irq);
void lw_UART_DisableIRQ(lw_uart* inst , lw_uart_irq_t irq);

#endif /* INC_LW_UART_H_ */
