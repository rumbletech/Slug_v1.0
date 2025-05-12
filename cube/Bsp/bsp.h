/*
 * bsp.h
 *
 *  Created on: May 13, 2025
 *      Author: rumbl
 */

#ifndef BSP_H_
#define BSP_H_

#include "stm32f1xx.h"

#define BSP_RGB_RED_LED_GPIO_PIN 4u
#define BSP_RGB_RED_LED_GPIO_PORT GPIOB

#define BSP_RGB_GREEN_LED_GPIO_PIN 5u
#define BSP_RGB_GREEN_LED_GPIO_PORT GPIOB

#define BSP_RGB_BLUE_LED_GPIO_PIN 3u
#define BSP_RGB_BLUE_LED_GPIO_PORT GPIOB

#define BSP_SDC_SPI_MOSI_PIN 5u
#define BSP_SDC_SPI_MOSI_PORT GPIOA

#define BSP_SDC_SPI_MISO_PIN 6u
#define BSP_SDC_SPI_MISO_PORT GPIOA

#define BSP_SDC_SPI_SCLK_PIN 7u
#define BSP_SDC_SPI_SCLK_PORT GPIOA

#define BSP_SDC_SPI_CS_PIN 4u
#define BSP_SDC_SPI_CS_PORT GPIOA

#define BSP_SDC_SPI SPI1

#endif /* BSP_H_ */
