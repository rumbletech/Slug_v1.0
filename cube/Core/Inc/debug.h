/*
 * debug.h
 *
 *  Created on: Oct 5, 2024
 *      Author: Mohamed-Rashad
 */

#ifndef INC_DEBUG_H_
#define INC_DEBUG_H_

#include "opts.h"

#if defined(_OPTS_DEBUG_EN) && _OPTS_DEBUG_EN == true

#include "main.h"

#define _DEBUG_PRINTF_TIMEOUT_      500u
#define _DEBUG_PRINTF_BUFFLEN_      128u
#define _DEUBG_PRINTF_ERRH_         1u
#define _DEBUG_PRINTF_BAUD_         115200u
#define _DEBUG_PRINTF_PARITY_       UART_PARITY_NONE
#define _DEBUG_PRINTF_PORT_         USART1
#define _DEBUG_PRINTF_MODE_         UART_MODE_TX_RX
#define _DEBUG_PRINTF_FLOW_         UART_HWCONTROL_NONE
#define _DEBUG_PRINTF_OVS_          UART_OVERSAMPLING_16
#define _DEBUG_PRINTF_STOPB_        UART_STOPBITS_1
#define _DEBUG_PRINTF_WLEN_         UART_WORDLENGTH_8B

extern void Debug_Init(UART_HandleTypeDef * huart );
extern int Debug_Printf (char * str, ...);
#endif

#endif /* INC_DEBUG_H_ */
