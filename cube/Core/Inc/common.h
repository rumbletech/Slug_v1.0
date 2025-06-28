/*
 * common.h
 *
 *  Created on: Oct 5, 2024
 *      Author: Garmoosh
 */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#include "opts.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define _UNUSED_(PARAM) ((void)PARAM)

typedef enum {
 E_OK,
 E_NOT_OK,
 E_PENDING,
 E_LENGTH,
} ProcStatus_t;

typedef struct {
	uint8_t* ptr;
	uint32_t len;
} auint8_t ;


#if defined(_OPTS_DEBUG_EN) && _OPTS_DEBUG_EN == true
#include "debug.h"
#define Common_Printf(...) Debug_Printf(__VA_ARGS__)
#else
#define Common_Printf(...) (_UNUSED_(0u))
#endif


#endif /* INC_COMMON_H_ */
