/*
 * rgb.c
 *
 *  Created on: Apr 20, 2025
 *      Author: rumbl
 */

#include "rgb.h"
#include "lw_gpio.h"
#include "bsp.h"

#define COLOR_R_BITPOS 2u
#define COLOR_G_BITPOS 1u
#define COLOR_B_BITPOS 0u

struct {
	lw_gpio gpio_led_r;
	lw_gpio gpio_led_g;
	lw_gpio gpio_led_b;
	uint8_t color;
} rgb ;

static void RGB_PreInit( void ){

	lw_RCC_Enable_GPIOB();
}

static void RGB_BSP_Init( void ){

	rgb.gpio_led_r.data.pin = BSP_RGB_RED_LED_GPIO_PIN;
	rgb.gpio_led_r.data.cfg = LW_GPIO_CFG_OUTPUT_PP;
	rgb.gpio_led_r.data.mode = LW_GPIO_MODE_10MHZ;
    rgb.gpio_led_r.hwctx = BSP_RGB_RED_LED_GPIO_PORT;
    lw_GPIO_Init(&rgb.gpio_led_r);

	rgb.gpio_led_g.data.pin = BSP_RGB_GREEN_LED_GPIO_PIN;
	rgb.gpio_led_g.data.cfg = LW_GPIO_CFG_OUTPUT_PP;
	rgb.gpio_led_g.data.mode = LW_GPIO_MODE_10MHZ;
    rgb.gpio_led_g.hwctx = BSP_RGB_GREEN_LED_GPIO_PORT;
    lw_GPIO_Init(&rgb.gpio_led_g);

	rgb.gpio_led_b.data.pin = BSP_RGB_BLUE_LED_GPIO_PIN;
	rgb.gpio_led_b.data.cfg = LW_GPIO_CFG_OUTPUT_PP;
	rgb.gpio_led_b.data.mode = LW_GPIO_MODE_10MHZ;
    rgb.gpio_led_b.hwctx = BSP_RGB_BLUE_LED_GPIO_PORT;
    lw_GPIO_Init(&rgb.gpio_led_b);
}

void RGB_Write( uint8_t color ){

	lw_GPIO_Write(&rgb.gpio_led_r, (color&(1u << COLOR_R_BITPOS))?(1u):(0u));
	lw_GPIO_Write(&rgb.gpio_led_g, (color&(1u << COLOR_G_BITPOS))?(1u):(0u));
	lw_GPIO_Write(&rgb.gpio_led_b, (color&(1u << COLOR_B_BITPOS))?(1u):(0u));
	rgb.color = color;
}

void RGB_Init( void ){

	RGB_PreInit();
	RGB_BSP_Init();
    RGB_Write(COLOR_BLACK);
}
