/*
 * common.h
 *
 *  Created on: Oct 5, 2024
 *      Author: Garmoosh
 */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#include "opts.h"

#define _UNUSED_(PARAM) ((void)PARAM)


#if defined(_OPTS_DEBUG_EN) && _OPTS_DEBUG_EN == true
#define Common_Printf(...) Debug_Printf(__VA_ARGS__)
#else
#define Common_Printf(...) (_UNUSED_(0u))
#endif


#endif /* INC_COMMON_H_ */
