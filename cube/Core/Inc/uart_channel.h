/*
 * uart_channel.h
 *
 *  Created on: Apr 25, 2025
 *      Author: rumbl
 */

#ifndef INC_UART_CHANNEL_H_
#define INC_UART_CHANNEL_H_

#include <stdint.h>
#include <stdbool.h>

#define CHANNEL_1 1u
#define CHANNEL_2 2u
#define CHANNEL_3 3u

typedef enum {
	e_uch_parity_none = 0,
	e_uch_parity_odd,
	e_uch_parity_even,
	e_uch_parity_MAX_,

} e_uch_parity;

typedef enum {
	e_uch_nStopbits_1 = 0,
	e_uch_nStopbits_2,
	e_uch_nStopbits_MAX_,
} e_uch_nStopbits;

typedef enum {
	e_uch_nDatabits_8 = 0,
	e_uch_nDatabits_9,
	e_uch_nDatabits_MAX_,

} e_uch_nDatabits;

typedef struct uch_config_s {
	uint32_t baudRate;
	e_uch_parity parity;
	e_uch_nStopbits nStopBits;
	e_uch_nDatabits nDataBits;

} uch_config;

extern void uch_Init ( void );
extern void uch_EnumerateChannels( void );
extern void uch_ApplyConfiguration ( uint8_t channelID );

#endif /* INC_UART_CHANNEL_H_ */
