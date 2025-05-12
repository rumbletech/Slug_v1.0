/*
 * lw_spi.c
 *
 *  Created on: May 13, 2025
 *      Author: rumbl
 */


#include "lw_spi.h"

/**
 * @brief Initializes the SPI peripheral and associated GPIO pins.
 * @param inst Pointer to lw_spi structure containing SPI and GPIO configuration.
 */
void lw_SPI_Init(lw_spi* inst) {
    inst->hwctx->CR1 = 0; // Clear CR1
    inst->hwctx->CR1 |= (0x3 << 3); // Baud rate = fPCLK/16 (011)
    inst->hwctx->CR1 |= SPI_CR1_MSTR; // Master mode
    inst->hwctx->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI; // Software slave management, internal NSS=1
    inst->hwctx->CR1 |= SPI_CR1_SPE; // Enable SPI
}

/**
 * @brief Transmits and receives data over SPI.
 * @param inst Pointer to lw_spi structure.
 * @param txptr Pointer to transmit buffer (can be NULL if only receiving).
 * @param rxptr Pointer to receive buffer (can be NULL if only transmitting).
 * @param len Number of bytes to transfer.
 */
void lw_SPI_TransmitReceieve(lw_spi* inst, uint8_t* txptr, uint8_t* rxptr, uint16_t len) {
    uint16_t i;
    uint8_t dummy = 0xFF;

    for (i = 0; i < len; i++) {
        while (!(inst->hwctx->SR & SPI_SR_TXE));

        inst->hwctx->DR = (txptr != NULL) ? txptr[i] : dummy;

        while (!(inst->hwctx->SR & SPI_SR_RXNE));

        uint8_t rxdata = inst->hwctx->DR;
        if (rxptr != NULL) {
            rxptr[i] = rxdata;
        }
    }

    while (inst->hwctx->SR & SPI_SR_BSY);
}
