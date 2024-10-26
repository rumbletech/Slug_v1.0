/*
 * SDC_SPI.c
 *
 *  Created on: Oct 26, 2024
 *      Author: pc
 */

/* Sets Clock Pre-Scale and Re-Initiliazes the Driver */

_sdcd_err SPI_SetClk ( SPI_HandleTypeDef* spiHandle , uint32_t preScale ){
	_sdcd_err err = SDCD_SUCCESS;

	spiHandle->Init.BaudRatePrescaler = preScale;
	HAL_StatusTypeDef ret = HAL_SPI_Init(spiHandle);

	if ( ret != HAL_OK ){
		err = SDCD_HAL_FAIL;
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

_sdcd_err SPI_Tx(SPI_HandleTypeDef* spiHandle , uint8_t* tdata , uint16_t len){
	_sdcd_err err = SDCD_SUCCESS;

	while(!__HAL_SPI_GET_FLAG(spiHandle , SPI_FLAG_TXE));
	HAL_StatusTypeDef hal_ret = HAL_SPI_Transmit(sdcd.spi_d, tdata, len , SPI_TIMEOUT);

	if ( hal_ret != HAL_OK ){
		err = SDCD_HAL_FAIL;
		if ( hal_ret == HAL_TIMEOUT ){
			err = SDCD_HAL_TIMEOUT;
		}
	}

	return err;
}

/* Transmits and Receives Bytes , both buffers must be allocated at least *len* bytes*/

_sdcd_err SPI_TRx(SPI_HandleTypeDef* spiHandle  , uint8_t * tdata , uint8_t* rdata , uint16_t len){
	_sdcd_err err = SDCD_SUCCESS;

	while(!__HAL_SPI_GET_FLAG(sdcd.spi_d, SPI_FLAG_TXE));
	HAL_StatusTypeDef hal_ret = HAL_SPI_TransmitReceive(spiHandle , tdata , rdata , len , SPI_TIMEOUT);

	if ( hal_ret != HAL_OK ){
		err = SDCD_HAL_FAIL;
		if ( hal_ret == HAL_TIMEOUT ){
			err = SDCD_HAL_TIMEOUT;
		}
	}

	return err;
}

/* Receives a number of bytes in master mode , MOSI bytes are 0xFF , ... */

_sdcd_err SPI_Rx(SPI_HandleTypeDef* spiHandle , uint8_t* rdata , uint16_t len){
	_sdcd_err err = SDCD_SUCCESS;
	uint8_t dummy = 0xFF;

	while ( len-- ){
		err = SPI_TRx(spiHandle , &dummy, rdata++ , 1u);
		if ( err != SDCD_SUCCESS ){
			break;
		}
	}
	return err;
}


