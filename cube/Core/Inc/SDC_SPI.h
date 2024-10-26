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

#define SDC_SPI_TIMEOUT_MS 10u

#define SDC_SPI_SUCCESS 0x0000
#define SDC_SPI_FAILURE 0x0001
#define SDC_SPI_TIMEOUT 0x0002

typedef uint16_t _sdc_spi_err;

_sdc_spi_err SPI_SetClk ( SPI_HandleTypeDef* spiHandle , uint32_t preScale );
void SPI_CS_Enable ( GPIO_HandleTypeDef* gpioHandle );
void SPI_CS_Disable ( GPIO_HandleTypeDef* gpioHandle );
_sdc_spi_err SPI_Tx(SPI_HandleTypeDef* spiHandle , uint8_t* tdata , uint16_t len);
_sdc_spi_err SPI_TRx(SPI_HandleTypeDef* spiHandle  , uint8_t * tdata , uint8_t* rdata , uint16_t len);
_sdc_spi_err SPI_Rx(SPI_HandleTypeDef* spiHandle , uint8_t* rdata , uint16_t len);

#endif /* INC_SDC_SPI_H_ */
