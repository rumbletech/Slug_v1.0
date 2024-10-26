/*
 * SDC_SPI.h
 *
 *  Created on: Oct 26, 2024
 *      Author: pc
 */

#ifndef INC_SDC_SPI_H_
#define INC_SDC_SPI_H_

#include "GPIO.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>

_sdcd_err SPI_SetClk ( SPI_HandleTypeDef* spiHandle , uint32_t preScale );
void SPI_CS_Enable ( GPIO_HandleTypeDef* gpioHandle );
void SPI_CS_Disable ( GPIO_HandleTypeDef* gpioHandle );
_sdcd_err SPI_Tx(SPI_HandleTypeDef* spiHandle , uint8_t* tdata , uint16_t len);
_sdcd_err SPI_TRx(SPI_HandleTypeDef* spiHandle  , uint8_t * tdata , uint8_t* rdata , uint16_t len);
_sdcd_err SPI_Rx(SPI_HandleTypeDef* spiHandle , uint8_t* rdata , uint16_t len);


#endif /* INC_SDC_SPI_H_ */
