/*
 * rgb.c
 *
 *  Created on: Apr 20, 2025
 *      Author: rumbl
 */


#include "rgb.h"
#include "GPIO.h"

#define COLOR_R_BITPOS 2u
#define COLOR_G_BITPOS 1u
#define COLOR_B_BITPOS 0u

GPIO_HandleTypeDef gpio_led_r;
GPIO_HandleTypeDef gpio_led_g;
GPIO_HandleTypeDef gpio_led_b;

void RGB_Init( void ){

	gpio_led_r.pinData.Pin = GPIO_PIN_5;
	gpio_led_r.pinData.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_led_r.pinData.Pull = GPIO_NOPULL;
    gpio_led_r.pinData.Speed = GPIO_SPEED_FREQ_LOW;
    gpio_led_r.port = GPIOB;
    gpio_led_r.pinNum = 5u;

    HAL_GPIO_Init(gpio_led_r.port, &gpio_led_r.pinData);

	gpio_led_g.pinData.Pin = GPIO_PIN_4;
	gpio_led_g.pinData.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_led_g.pinData.Pull = GPIO_NOPULL;
    gpio_led_g.pinData.Speed = GPIO_SPEED_FREQ_LOW;
    gpio_led_g.port = GPIOB;
    gpio_led_g.pinNum = 4u;

    HAL_GPIO_Init(gpio_led_g.port, &gpio_led_g.pinData);


	gpio_led_b.pinData.Pin = GPIO_PIN_3;
	gpio_led_b.pinData.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_led_b.pinData.Pull = GPIO_NOPULL;
    gpio_led_b.pinData.Speed = GPIO_SPEED_FREQ_LOW;
    gpio_led_b.port = GPIOB;
    gpio_led_b.pinNum = 3u;

    HAL_GPIO_Init(gpio_led_b.port, &gpio_led_b.pinData);

}

void RGB_Write( uint8_t color ){
	HAL_GPIO_WritePin(gpio_led_r.port, gpio_led_r.pinData.Pin , (color&(1u << COLOR_R_BITPOS))?(GPIO_PIN_SET):(GPIO_PIN_RESET));
	HAL_GPIO_WritePin(gpio_led_g.port, gpio_led_g.pinData.Pin , (color&(1u << COLOR_G_BITPOS))?(GPIO_PIN_SET):(GPIO_PIN_RESET));
	HAL_GPIO_WritePin(gpio_led_b.port, gpio_led_b.pinData.Pin , (color&(1u << COLOR_B_BITPOS))?(GPIO_PIN_SET):(GPIO_PIN_RESET));
}
