/*
 * lw_gpio.c
 *
 *  Created on: May 13, 2025
 *      Author: rumbl
 */

#include "..\Inc\lw_gpio.h"

/**
 * @brief Initializes the GPIO pin as output push-pull with no pull-up/pull-down.
 * @param inst Pointer to lw_gpio structure containing GPIO port and pin number.
 */
void lw_GPIO_Init(lw_gpio* inst) {

    // Determine if pin is in CRL (pins 0-7) or CRH (pins 8-15).
    if (inst->data.pin < 8) {
        // Configure CRL for pins 0-7.
        uint32_t shift = inst->data.pin * 4; // Each pin uses 4 bits.
        inst->hwctx->CRL &= ~(0xFU << shift); // Clear mode and CNF bits.
        inst->hwctx->CRL |= (0x3U << shift);  // Mode: 11 (50 MHz output), CNF: 00 (push-pull).
    } else {
        // Configure CRH for pins 8-15.
        uint32_t shift = (inst->data.pin - 8) * 4; // Adjust for CRH.
        inst->hwctx->CRH &= ~(0xFU << shift); // Clear mode and CNF bits.
        inst->hwctx->CRH |= (0x3U << shift);  // Mode: 11 (50 MHz output), CNF: 00 (push-pull).
    }

    inst->hwctx->ODR &= ~(1U << inst->data.pin); // Clear output pin.
}

/**
 * @brief Sets the GPIO pin to a state.
 * @param inst Pointer to lw_gpio structure containing GPIO port and pin number.
 */
void lw_GPIO_Write(lw_gpio* inst , uint8_t state) {
	if ( state ){
		inst->hwctx->BSRR = (1U << inst->data.pin); // Set pin high using Bit Set/Reset Register.
	}
	else{
	    inst->hwctx->BSRR = (1U << (inst->data.pin + 16)); // Clear pin using BSRR high-order bits.
	}
}
