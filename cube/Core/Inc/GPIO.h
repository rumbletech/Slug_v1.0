/*
 * GPIO.h
 *
 *  Created on: Oct 6, 2024
 *      Author: pc
 */

#ifndef INC_GPIO_H_
#define INC_GPIO_H_


typedef struct {

	GPIO_InitTypeDef pinData;
	uint8_t pinNum;
	GPIO_TypeDef * port;

} GPIO_HandleTypeDef;


#endif /* INC_GPIO_H_ */
