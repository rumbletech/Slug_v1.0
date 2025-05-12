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


typedef struct lw_gpio_data_s {
	uint8_t pin;
}lw_gpio_data;

typedef struct lw_gpio_s {

	GPIO_TypeDef* hwctx;
	lw_gpio_data data;

} lw_gpio ;


void lw_GPIO_Init(lw_gpio* inst);
void lw_GPIO_Write(lw_gpio* inst , uint8_t state);

#endif /* INC_LW_GPIO_H_ */
