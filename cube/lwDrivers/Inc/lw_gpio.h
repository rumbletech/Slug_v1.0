/*
 * lw_gpio.h
 *
 *  Created on: May 13, 2025
 *      Author: rumbl
 */

#ifndef INC_LW_GPIO_H_
#define INC_LW_GPIO_H_


#include <stdint.h>
#include "common.h"
#include "stm32f1xx.h"


// Enum for GPIO mode (CNF bits), ordered: input modes, output modes, alternate function modes
typedef enum {
    LW_GPIO_CFG_ANALOG,              // Analog mode (input)
    LW_GPIO_CFG_INPUT_FLOAT,         // Floating input
    LW_GPIO_CFG_INPUT_PULL,          // Input with pull-up/pull-down
    LW_GPIO_CFG_OUTPUT_PP,           // Output push-pull
    LW_GPIO_CFG_OUTPUT_OD,           // Output open-drain
    LW_GPIO_CFG_ALT_PP,              // Alternate function push-pull
    LW_GPIO_CFG_ALT_OD               // Alternate function open-drain
} lw_gpio_cfg_t;

// Enum for GPIO speed (MODE bits), ordered: input, then increasing speed
typedef enum {
	LW_GPIO_MODE_INPUT,             // Input  mode
    LW_GPIO_MODE_2MHZ,               // Output mode, 2 MHz
    LW_GPIO_MODE_10MHZ,              // Output mode, 10 MHz
    LW_GPIO_MODE_50MHZ               // Output mode, 50 MHz
} lw_gpio_mode_t;

// Structure for GPIO configuration data
typedef struct {
    uint8_t pin;                      // Pin number (0-15)
    lw_gpio_cfg_t cfg;              // GPIO mode (CNF bits)
    lw_gpio_mode_t mode;            // GPIO speed (MODE bits)
} lw_gpio_data_t;

typedef struct lw_gpio_s {

	GPIO_TypeDef* hwctx;
	lw_gpio_data_t data;

} lw_gpio ;


void lw_GPIO_Init(lw_gpio* inst);
void lw_GPIO_Write(lw_gpio* inst , uint8_t state);

#endif /* INC_LW_GPIO_H_ */
