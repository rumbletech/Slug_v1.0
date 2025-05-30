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
#include "lw_uart.h"

#define _DEBUG_PRINTF_TIMEOUT_      500u
#define _DEBUG_PRINTF_BUFFLEN_      2048u
#define _DEUBG_PRINTF_ERRH_         1u

extern void Debug_Init(lw_uart* huart );
extern int Debug_Printf (char * str, ...);

#endif
#endif /* INC_DEBUG_H_ */
