/*
 * uart_channel.h
 *
 *  Created on: Apr 25, 2025
 *      Author: rumbl
 */

#ifndef INC_COM_CHANNEL_H_
#define INC_COM_CHANNEL_H_

#include "common.h"
#include "lw_uart.h"

typedef enum { COM_CHANNEL_1,
			   COM_CHANNEL_2,
			   COM_CHANNEL_3,
			   COM_CHANNEL_LENGTH
} e_Com_Channel_ID;

/* IRQ Handler for Channels , Called for Each IRQ for each Uart Dev */
extern void Com_Channel_IRQnHandler( USART_TypeDef* hwctx );
/* Initialize the COM Channels */
extern void Com_Channel_Init( void );
/* Completely Disables the Channel -> Power OFF */
extern void Com_Channel_Disable( uint8_t comID );
/* Enables the Channel -> Power ON  */
extern void Com_Channel_Enable( uint8_t comID );
/* Configures the Channel Parameters */
extern void Com_Channel_Configure( uint8_t comID , lw_uart_data_t cfg);
/* Starts Logging on The Channel */
extern void Com_Channel_StartLogging( uint8_t comID );
/* Stops Logging on The Channel */
extern void Com_Channel_StopLogging( uint8_t comID );
/* returns the data from the com module */
extern auint8_t Com_Channel_Read( uint8_t comID );
/* Returns Error Conditions on the com channel */
extern uint32_t Com_Channel_GetErrors( uint8_t comID  );

#endif /* INC_COM_CHANNEL_H_ */
