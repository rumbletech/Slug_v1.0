/*
 * lw_spi.h
 *
 *  Created on: May 13, 2025
 *      Author: rumbl
 */

#ifndef INC_LW_SPI_H_
#define INC_LW_SPI_H_

#include <stdint.h>
#include <stm32f1xx.h>
#include "common.h"

typedef struct {
	SPI_TypeDef* hwctx;
} lw_spi;

void lw_SPI_Init(lw_spi* inst);
void lw_SPI_TransmitReceieve(lw_spi* inst, uint8_t* txptr, uint8_t* rxptr, uint16_t len);

#endif /* INC_LW_SPI_H_ */
