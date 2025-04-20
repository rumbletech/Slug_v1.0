/*
 * rgb.h
 *
 *  Created on: Apr 20, 2025
 *      Author: rumbl
 */

#ifndef INC_RGB_H_
#define INC_RGB_H_

#include <stdint.h>

/* RGB Codes */
#define COLOR_BLACK   0b000
#define COLOR_BLUE    0b001
#define COLOR_GREEN   0b010
#define COLOR_CAYAN   0b011
#define COLOR_RED     0b100
#define COLOR_MAGNETA 0b101
#define COLOR_YELLOW  0b110
#define COLOR_WHITE   0b111


void RGB_Init( void );
void RGB_Write( uint8_t color );

#endif /* INC_RGB_H_ */
