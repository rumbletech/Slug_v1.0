/*
 * uart_channel.c
 *
 *  Created on: Apr 25, 2025
 *      Author: rumbl
 */


#include "com_channel.h"
#include "lw_sys.h"
#include "lw_rcc.h"
#include "lw_gpio.h"
#include "sfifo.h"
#include "bsp.h"
#include <string.h>

#define RX_BUFFER_SIZE 512U


struct Com_Channel_Phy_s {
	lw_gpio gpioDev_Enable;
	lw_gpio gpioDev_Tx;
	lw_gpio gpioDev_Rx;
	lw_uart uartDev;
};

struct Com_Channel_s {

	/* Physical Conext of Channel */
	struct Com_Channel_Phy_s phy;
	struct ppbf_s pingPong;

};

static const uint8_t com_channel_init_gpio_enable_pins[COM_CHANNEL_LENGTH] = {BSP_CH1_EN_PIN,BSP_CH2_EN_PIN,BSP_CH3_EN_PIN};
static GPIO_TypeDef* const com_channel_init_gpio_enable_ports[COM_CHANNEL_LENGTH] = {BSP_CH1_EN_PORT,BSP_CH2_EN_PORT,BSP_CH3_EN_PORT};

static const uint8_t com_channel_init_gpio_tx_pins[COM_CHANNEL_LENGTH] = {BSP_CH1_UART_TX_PIN,BSP_CH2_UART_TX_PIN,BSP_CH3_UART_TX_PIN};
static GPIO_TypeDef* const com_channel_init_gpio_tx_ports[COM_CHANNEL_LENGTH] = {BSP_CH1_UART_TX_PORT,BSP_CH2_UART_TX_PORT,BSP_CH3_UART_TX_PORT};

static const uint8_t com_channel_init_gpio_rx_pins[COM_CHANNEL_LENGTH] = {BSP_CH1_UART_RX_PIN,BSP_CH2_UART_RX_PIN,BSP_CH3_UART_RX_PIN};
static GPIO_TypeDef* const com_channel_init_gpio_rx_ports[COM_CHANNEL_LENGTH] = {BSP_CH1_UART_RX_PORT,BSP_CH2_UART_RX_PORT,BSP_CH3_UART_RX_PORT};

static USART_TypeDef* const com_channel_init_uart[COM_CHANNEL_LENGTH] = {BSP_CH1_UART,BSP_CH2_UART,BSP_CH3_UART};

static const uint32_t com_channel_init_irqn[COM_CHANNEL_LENGTH] = {USART3_IRQn,USART2_IRQn,USART1_IRQn};
static const uint32_t com_channel_init_irqn_prios[COM_CHANNEL_LENGTH] = {3u,4u,5u};


static struct Com_Channel_s COM_Channels[COM_CHANNEL_LENGTH];
static uint8_t COM_Channels_RxBuffer[COM_CHANNEL_LENGTH][2U][RX_BUFFER_SIZE];


static void Com_Channel_PreInit( void ){
	lw_RCC_Enable_GPIOA();
	lw_RCC_Enable_GPIOB();
	lw_RCC_Enable_USART1();
	lw_RCC_Enable_USART2();
	lw_RCC_Enable_USART3();
}

extern void Com_Channel_Enable ( uint8_t comID ){
    lw_GPIO_Write(&COM_Channels[comID].phy.gpioDev_Enable,1u);
}

extern void Com_Channel_Disable ( uint8_t comID ){
    lw_GPIO_Write(&COM_Channels[comID].phy.gpioDev_Enable,0u);
}

static void Com_Channel_BSP_Init( void ){

	for ( uint8_t i = 0 ; i < COM_CHANNEL_LENGTH ; i++ ){

		/* Init UART TX Pin GPIO Device  */
		COM_Channels[i].phy.gpioDev_Tx.hwctx = com_channel_init_gpio_tx_ports[i];
		COM_Channels[i].phy.gpioDev_Tx.data.pin = com_channel_init_gpio_tx_pins[i];
		COM_Channels[i].phy.gpioDev_Tx.data.cfg = LW_GPIO_CFG_ALT_PP;
		COM_Channels[i].phy.gpioDev_Tx.data.mode = LW_GPIO_MODE_50MHZ;
	    lw_GPIO_Init(&COM_Channels[i].phy.gpioDev_Tx);

		/* Init UART RX Pin GPIO Device  */
	    COM_Channels[i].phy.gpioDev_Rx.hwctx = com_channel_init_gpio_rx_ports[i];
	    COM_Channels[i].phy.gpioDev_Rx.data.pin = com_channel_init_gpio_rx_pins[i];
	    COM_Channels[i].phy.gpioDev_Rx.data.cfg = LW_GPIO_CFG_INPUT_FLOAT;
	    COM_Channels[i].phy.gpioDev_Rx.data.mode = LW_GPIO_MODE_INPUT;
	    lw_GPIO_Init(&COM_Channels[i].phy.gpioDev_Rx);

		/* Init UART Device  */
	    COM_Channels[i].phy.uartDev.hwctx = com_channel_init_uart[i];
	    COM_Channels[i].phy.uartDev.data.baudRate = 115200;
	    COM_Channels[i].phy.uartDev.data.dataBits = LW_UART_NUM_DATA_BITS_8;
	    COM_Channels[i].phy.uartDev.data.stopBits = LW_UART_NUM_STOP_BITS_1;
	    COM_Channels[i].phy.uartDev.data.parity = LW_UART_PARITY_NONE;
		lw_UART_Init(&COM_Channels[i].phy.uartDev);

		lw_Sys_IRQ_Set_Priority(com_channel_init_irqn[i],com_channel_init_irqn_prios[i],0u);
		lw_Sys_IRQ_Enable(com_channel_init_irqn[i]);

		/* Init Enable Pin GPIO Device */
		COM_Channels[i].phy.gpioDev_Enable.hwctx = com_channel_init_gpio_enable_ports[i];
		COM_Channels[i].phy.gpioDev_Enable.data.pin = com_channel_init_gpio_enable_pins[i];
		COM_Channels[i].phy.gpioDev_Enable.data.cfg = LW_GPIO_CFG_OUTPUT_PP;
		COM_Channels[i].phy.gpioDev_Enable.data.mode = LW_GPIO_MODE_10MHZ;
	    lw_GPIO_Init(&COM_Channels[i].phy.gpioDev_Enable);

		/* Disable The Channel */
	    lw_GPIO_Write(&COM_Channels[i].phy.gpioDev_Enable,0u);
	}
}

extern void Com_Channel_Init ( void ){

	Com_Channel_PreInit();

	Com_Channel_BSP_Init();

#if defined(_OPTS_DEBUG_EN) && _OPTS_DEBUG_EN == true
	Debug_Init(&COM_Channels[COM_CHANNEL_1].phy.uartDev);
	Com_Channel_Enable(COM_CHANNEL_1);
#endif

	/* Init Fifos */
	for ( uint8_t i = 0 ; i < COM_CHANNEL_LENGTH ; i++ ){
		ppbf_init(&COM_Channels[i].pingPong,&COM_Channels_RxBuffer[i][0u][0u],&COM_Channels_RxBuffer[i][1u][0u],RX_BUFFER_SIZE,RX_BUFFER_SIZE);
	}
}

extern void Com_Channel_Configure ( uint8_t comID , lw_uart_data_t cfg ){


	/* Core Reset Here */

	/* Apply UART Configuration */

	COM_Channels[comID].phy.uartDev.data.baudRate = cfg.baudRate;
	COM_Channels[comID].phy.uartDev.data.dataBits = cfg.dataBits;
	COM_Channels[comID].phy.uartDev.data.stopBits = cfg.stopBits;
	COM_Channels[comID].phy.uartDev.data.parity = cfg.parity;

	lw_UART_Init(&COM_Channels[comID].phy.uartDev);

}

extern void Com_Channel_StartLogging( uint8_t comID ){
	lw_UART_EnableIRQ(&COM_Channels[comID].phy.uartDev,LW_UART_IRQ_RXNE);
}

extern void Com_Channel_StopLogging( uint8_t comID ){
	lw_UART_DisableIRQ(&COM_Channels[comID].phy.uartDev,LW_UART_IRQ_RXNE);
}

extern void Com_Channel_IRQnHandler( USART_TypeDef* hwctx ){

	uint8_t comID = hwctx == com_channel_init_uart[COM_CHANNEL_1] ? COM_CHANNEL_1 :
			        hwctx == com_channel_init_uart[COM_CHANNEL_2] ? COM_CHANNEL_2 :
			        hwctx == com_channel_init_uart[COM_CHANNEL_3] ? COM_CHANNEL_3 : COM_CHANNEL_1;


	uint8_t data = lw_UART_Receieve(&COM_Channels[comID].phy.uartDev);

	Common_Printf("d[%c]\r\n",data);

	ppbf_write(&COM_Channels[comID].pingPong, &data, sizeof(data));


}

extern auint8_t Com_Channel_Read( uint8_t comID ){

	auint8_t ret;

	/* CRITICAL SECTION START */

	/* Switch Ping Pong Buffers , get the one that is actively being written to */
	ppbf_switch(&COM_Channels[comID].pingPong);

	/* Return the read pointer and length */
	ret.ptr = ppbf_get_read_ptr(&COM_Channels[comID].pingPong);
	ret.len = ppbf_get_read_len(&COM_Channels[comID].pingPong);

	/* CRITICAL SECTION END */


	return ret;
}


extern uint32_t Com_Channel_GetErrors( uint8_t comID  ){
	return ppbf_get_err(&COM_Channels[comID].pingPong);
}

