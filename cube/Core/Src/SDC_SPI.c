/*
 * SDC_SPI.c
 *
 *  Created on: Oct 26, 2024
 *      Author: Mohammad Rashad
 *      SDC Module SPI Driver.
 */

#include "SDC_SPI.h"

/* Sets Clock Pre-Scale and Re-Initiliazes the Driver */

_sdc_spi_err SPI_SetClk ( SPI_HandleTypeDef* spiHandle , uint32_t preScale ){
	_sdc_spi_err err = SDC_SPI_SUCCESS;

	spiHandle->Init.BaudRatePrescaler = preScale;
	HAL_StatusTypeDef ret = HAL_SPI_Init(spiHandle);

	if ( ret != HAL_OK ){
		err = SDC_SPI_FAILURE;
	}
	return err;
}

/* Enables CS , LOW */

void SPI_CS_Enable ( GPIO_HandleTypeDef* gpioHandle ){
	HAL_GPIO_WritePin(gpioHandle->port, 1u << gpioHandle->pinNum, GPIO_PIN_RESET);
}

/* Enables CS , High */

void SPI_CS_Disable ( GPIO_HandleTypeDef* gpioHandle ){
	HAL_GPIO_WritePin(gpioHandle->port, 1u << gpioHandle->pinNum, GPIO_PIN_SET);
}

/* Transmits Bytes and Ignores the MISO Received Bytes */

_sdc_spi_err SPI_Tx(SPI_HandleTypeDef* spiHandle , uint8_t* tdata , uint16_t len){
	_sdc_spi_err err = SDC_SPI_SUCCESS;

	while(!__HAL_SPI_GET_FLAG(spiHandle , SPI_FLAG_TXE));
	HAL_StatusTypeDef hal_ret = HAL_SPI_Transmit(spiHandle , tdata, len , SDC_SPI_TIMEOUT_MS);

	if ( hal_ret != HAL_OK ){
		err = SDC_SPI_FAILURE;
		if ( hal_ret == HAL_TIMEOUT ){
			err = SDC_SPI_TIMEOUT;
		}
	}
	return err;
}

/* Transmits and Receives Bytes , both buffers must be allocated at least *len* bytes*/

_sdc_spi_err SPI_TRx(SPI_HandleTypeDef* spiHandle  , uint8_t * tdata , uint8_t* rdata , uint16_t len){
	_sdc_spi_err err = SDC_SPI_SUCCESS;

	while(!__HAL_SPI_GET_FLAG(spiHandle , SPI_FLAG_TXE));
	HAL_StatusTypeDef hal_ret = HAL_SPI_TransmitReceive(spiHandle , tdata , rdata , len , SDC_SPI_TIMEOUT_MS);

	if ( hal_ret != HAL_OK ){
		err = SDC_SPI_FAILURE;
		if ( hal_ret == HAL_TIMEOUT ){
			err = SDC_SPI_TIMEOUT;
		}
	}

	return err;
}

/* Receives a number of bytes in master mode , MOSI bytes are 0xFF , ... */

_sdc_spi_err SPI_Rx(SPI_HandleTypeDef* spiHandle , uint8_t* rdata , uint16_t len){
	_sdc_spi_err err = SDC_SPI_SUCCESS;
	uint8_t dummy = 0xFF;

	while ( len-- ){
		err = SPI_TRx(spiHandle , &dummy, rdata++ , 1u);
		if ( err != SDC_SPI_SUCCESS ){
			break;
		}
	}
	return err;
}


