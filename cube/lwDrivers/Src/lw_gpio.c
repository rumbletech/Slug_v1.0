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
    uint32_t shift;
    uint32_t cnf_bits = 0;
    uint32_t mode_bits = 0;

    // Determine configuration register (CRL for pins 0-7, CRH for pins 8-15)
    if (inst->data.pin < 8U) {
        shift = inst->data.pin * 4U; // Each pin uses 4 bits (CNF[1:0], MODE[1:0])
        inst->hwctx->CRL &= ~(0xFU << shift); // Clear CNF and MODE bits
    } else {
        shift = (inst->data.pin - 8U) * 4U; // Adjust for CRH
        inst->hwctx->CRH &= ~(0xFU << shift); // Clear CNF and MODE bits
    }

    // Configure CNF bits based on mode
    switch (inst->data.cfg) {
        case LW_GPIO_CFG_ANALOG:
            cnf_bits = 0x0; // 00: Analog CFG
            break;
        case LW_GPIO_CFG_INPUT_FLOAT:
            cnf_bits = 0x1; // 01: Floating input
            break;
        case LW_GPIO_CFG_INPUT_PULL:
            cnf_bits = 0x2; // 10: Input with pull-up/pull-down
            break;
        case LW_GPIO_CFG_OUTPUT_PP:
            cnf_bits = 0x0; // 00: Output push-pull
            break;
        case LW_GPIO_CFG_OUTPUT_OD:
            cnf_bits = 0x1; // 01: Output open-drain
            break;
        case LW_GPIO_CFG_ALT_PP:
            cnf_bits = 0x2; // 10: Alternate function push-pull
            break;
        case LW_GPIO_CFG_ALT_OD:
            cnf_bits = 0x3; // 11: Alternate function open-drain
            break;
        default:
            cnf_bits = 0x1; // Default to floating input
            break;
    }

    // Configure MODE bits based on MODE
    switch (inst->data.mode) {
    	case LW_GPIO_MODE_INPUT:
    		mode_bits = 0x0;
    		break;
        case LW_GPIO_MODE_2MHZ:
            mode_bits = 0x2; // 10: 2 MHz
            break;
        case LW_GPIO_MODE_10MHZ:
            mode_bits = 0x1; // 01: 10 MHz
            break;
        case LW_GPIO_MODE_50MHZ:
            mode_bits = 0x3; // 11: 50 MHz
            break;
        default:
            mode_bits = 0x0; // Default to input mode
            break;
    }

    // Combine CNF and MODE bits (CNF[1:0] at bits 3:2, MODE[1:0] at bits 1:0)
    uint32_t config = (cnf_bits << 2U) | mode_bits;

	/* Configure PULL */
    if ( cnf_bits == 0x02U && mode_bits == 0x00U ){
    	if ( inst->data.pud == LW_GPIO_PUD_PULL_UP ){
    		inst->hwctx->ODR |= ( 1UL << inst->data.pin );
    	}
    	else{
    		inst->hwctx->ODR &= ~( 1UL << inst->data.pin );
    	}
    }

    // Apply configuration to CRL or CRH
    if (inst->data.pin < 8U) {
        inst->hwctx->CRL |= (config << shift);
    } else {
        inst->hwctx->CRH |= (config << shift);
    }
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

/**
 * @brief Reads a GPIO.
 * @param inst Pointer to lw_gpio structure containing GPIO port and pin number.
 */
bool lw_GPIO_Read(lw_gpio* inst) {
	return (bool)( inst->hwctx->IDR & ( 1u << inst->data.pin ));
}
