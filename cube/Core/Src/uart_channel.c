/*
 * uart_channel.c
 *
 *  Created on: Apr 25, 2025
 *      Author: rumbl
 */


#include "uart_channel.h"
#include "opts.h"
#include "common.h"
#include "file_sys.h"
#include "jsmn.h"
#include "lw_uart.h"
#include "lw_gpio.h"
#include "lw_sys.h"
#include "bsp.h"
#include <string.h>

#define UCH_MAX_NUM 3u
#define UCH_CONFIG_MAX_TOKEN_SIZE 64u


struct {

	struct {
		lw_gpio ch_Enable[UCH_MAX_NUM];
		lw_gpio ch_UartTx[UCH_MAX_NUM];
		lw_gpio ch_UartRx[UCH_MAX_NUM];
		lw_uart ch_Uart[UCH_MAX_NUM];
	} phy;

	struct {
		bool channel_enable[UCH_MAX_NUM];
		struct uch_config_s channel_config[UCH_MAX_NUM];
	}config;


	struct {
		jsmn_parser parser;
		jsmntok_t tokens[UCH_CONFIG_MAX_TOKEN_SIZE];
	}json;

} uch;

static void uch_PreInit( void ){
	lw_RCC_Enable_GPIOA();
	lw_RCC_Enable_GPIOB();
	lw_RCC_Enable_USART1();
	lw_RCC_Enable_USART2();
	lw_RCC_Enable_USART3();
}

static void uch_BSP_Init( void ){

	/* Channel [1] */

	uch.phy.ch_UartTx[0u].hwctx = BSP_CH1_UART_TX_PORT;
	uch.phy.ch_UartTx[0u].data.pin = BSP_CH1_UART_TX_PIN;
	uch.phy.ch_UartTx[0u].data.cfg = LW_GPIO_CFG_ALT_PP;
	uch.phy.ch_UartTx[0u].data.mode = LW_GPIO_MODE_50MHZ;
    lw_GPIO_Init(&uch.phy.ch_UartTx[0u]);

	uch.phy.ch_UartRx[0u].hwctx = BSP_CH1_UART_RX_PORT;
	uch.phy.ch_UartRx[0u].data.pin = BSP_CH1_UART_RX_PIN;
	uch.phy.ch_UartRx[0u].data.cfg = LW_GPIO_CFG_INPUT_FLOAT;
	uch.phy.ch_UartRx[0u].data.mode = LW_GPIO_MODE_INPUT;
    lw_GPIO_Init(&uch.phy.ch_UartRx[0u]);

	uch.phy.ch_Uart[0u].hwctx = BSP_CH1_UART ;
	uch.phy.ch_Uart[0u].data.baudRate = 115200;
	uch.phy.ch_Uart[0u].data.dataBits = LW_UART_NUM_DATA_BITS_8;
	uch.phy.ch_Uart[0u].data.stopBits = LW_UART_NUM_STOP_BITS_1;
	uch.phy.ch_Uart[0u].data.parity = LW_UART_PARITY_NONE;
	lw_UART_Init(&uch.phy.ch_Uart[0u]);

#if defined(_OPTS_DEBUG_EN) && _OPTS_DEBUG_EN == true
	Debug_Init(&uch.phy.ch_Uart[0u]);
#endif

	uch.phy.ch_Enable[0u].hwctx = BSP_CH1_EN_PORT;
	uch.phy.ch_Enable[0u].data.pin = BSP_CH1_EN_PIN;
	uch.phy.ch_Enable[0u].data.cfg = LW_GPIO_CFG_OUTPUT_PP;
	uch.phy.ch_Enable[0u].data.mode = LW_GPIO_MODE_10MHZ;
    lw_GPIO_Init(&uch.phy.ch_Enable[0u]);
#if defined(_OPTS_DEBUG_EN) && _OPTS_DEBUG_EN == true
    lw_GPIO_Write(&uch.phy.ch_Enable[0u], _INIT_DEBUG_EN);
#else
    lw_GPIO_Write(&uch.phy.ch_Enable[0u], 0u);
#endif


	/* Channel [2] */

	uch.phy.ch_UartTx[1u].hwctx = BSP_CH2_UART_TX_PORT;
	uch.phy.ch_UartTx[1u].data.pin = BSP_CH2_UART_TX_PIN;
	uch.phy.ch_UartTx[1u].data.cfg = LW_GPIO_CFG_ALT_PP;
	uch.phy.ch_UartTx[1u].data.mode = LW_GPIO_MODE_50MHZ;
    lw_GPIO_Init(&uch.phy.ch_UartTx[1u]);

	uch.phy.ch_UartRx[1u].hwctx = BSP_CH2_UART_RX_PORT;
	uch.phy.ch_UartRx[1u].data.pin = BSP_CH2_UART_RX_PIN;
	uch.phy.ch_UartRx[1u].data.cfg = LW_GPIO_CFG_INPUT_FLOAT;
	uch.phy.ch_UartRx[1u].data.mode = LW_GPIO_MODE_INPUT;
    lw_GPIO_Init(&uch.phy.ch_UartRx[1u]);

	uch.phy.ch_Uart[1u].hwctx = BSP_CH2_UART ;
	uch.phy.ch_Uart[1u].data.baudRate = 115200;
	uch.phy.ch_Uart[1u].data.dataBits = LW_UART_NUM_DATA_BITS_8;
	uch.phy.ch_Uart[1u].data.stopBits = LW_UART_NUM_STOP_BITS_1;
	uch.phy.ch_Uart[1u].data.parity = LW_UART_PARITY_NONE;
	lw_UART_Init(&uch.phy.ch_Uart[1u]);

	uch.phy.ch_Enable[1u].hwctx = BSP_CH2_EN_PORT;
	uch.phy.ch_Enable[1u].data.pin = BSP_CH2_EN_PIN;
	uch.phy.ch_Enable[1u].data.cfg = LW_GPIO_CFG_OUTPUT_PP;
	uch.phy.ch_Enable[1u].data.mode = LW_GPIO_MODE_10MHZ;
    lw_GPIO_Init(&uch.phy.ch_Enable[1u]);
    lw_GPIO_Write(&uch.phy.ch_Enable[1u], 0u);


	/* Channel [3] */

	uch.phy.ch_UartTx[2u].hwctx = BSP_CH3_UART_TX_PORT;
	uch.phy.ch_UartTx[2u].data.pin = BSP_CH3_UART_TX_PIN;
	uch.phy.ch_UartTx[2u].data.cfg = LW_GPIO_CFG_ALT_PP;
	uch.phy.ch_UartTx[2u].data.mode = LW_GPIO_MODE_50MHZ;
    lw_GPIO_Init(&uch.phy.ch_UartTx[2u]);

	uch.phy.ch_UartRx[2u].hwctx = BSP_CH3_UART_RX_PORT;
	uch.phy.ch_UartRx[2u].data.pin = BSP_CH3_UART_RX_PIN;
	uch.phy.ch_UartRx[2u].data.cfg = LW_GPIO_CFG_INPUT_FLOAT;
	uch.phy.ch_UartRx[2u].data.mode = LW_GPIO_MODE_INPUT;
    lw_GPIO_Init(&uch.phy.ch_UartRx[2u]);

	uch.phy.ch_Uart[2u].hwctx = BSP_CH3_UART ;
	uch.phy.ch_Uart[2u].data.baudRate = 115200;
	uch.phy.ch_Uart[2u].data.dataBits = LW_UART_NUM_DATA_BITS_8;
	uch.phy.ch_Uart[2u].data.stopBits = LW_UART_NUM_STOP_BITS_1;
	uch.phy.ch_Uart[2u].data.parity = LW_UART_PARITY_NONE;
	lw_UART_Init(&uch.phy.ch_Uart[2u]);

	uch.phy.ch_Enable[2u].hwctx = BSP_CH3_EN_PORT;
	uch.phy.ch_Enable[2u].data.pin = BSP_CH3_EN_PIN;
	uch.phy.ch_Enable[2u].data.cfg = LW_GPIO_CFG_OUTPUT_PP;
	uch.phy.ch_Enable[2u].data.mode = LW_GPIO_MODE_10MHZ;
    lw_GPIO_Init(&uch.phy.ch_Enable[2u]);
    lw_GPIO_Write(&uch.phy.ch_Enable[2u], 0u);

}

extern void uch_Init ( void ){

	uch_PreInit();

	uch_BSP_Init();

	for ( uint8_t i = 0 ; i < UCH_MAX_NUM ; i++ ){
		uch.config.channel_enable[i] = false;
		uch.config.channel_config[i].baudRate = UINT32_MAX;
		uch.config.channel_config[i].nDataBits = UINT8_MAX;
		uch.config.channel_config[i].nStopBits = UINT8_MAX;
		uch.config.channel_config[i].parity = UINT8_MAX;
	}

	/* Init json parser */
	jsmn_init(&uch.json.parser);
}

static bool assertBaud( uint32_t baudRate ){
	const unsigned int supported_baud_rates[] = {
	    1200,
	    2400,
	    4800,
	    9600,
	    14400,
	    19200,
	    28800,
	    38400,
	    57600,
	    115200,
	    128000,
	    230400,
	    250000,
	    460800,
	    500000
	};
	bool ret = false;
	for ( uint32_t i = 0 ; i < sizeof(supported_baud_rates)/sizeof(unsigned int) ;i++ ){
		if ( baudRate == supported_baud_rates[i]){
			ret = true;
			break;
		}
	}
	return ret;
}

static bool assertConfiguration( uint8_t channelID ){

	if ( !assertBaud(uch.config.channel_config[channelID-1].baudRate)  ){
		return false;
	}

	if ( uch.config.channel_config[channelID-1].parity >= e_uch_parity_MAX_ ){
		return false;
	}

	if ( uch.config.channel_config[channelID-1].nStopBits >= e_uch_nStopbits_MAX_ ){
		return false;
	}

	if ( uch.config.channel_config[channelID-1].nDataBits >= e_uch_nDatabits_MAX_ ){
		return false;
	}

	return true;
}

extern struct uch_config_s uch_GetChannelConfiguration ( uint8_t channelID ){
	return uch.config.channel_config[channelID-1];
}

extern bool uch_GetChannelState ( uint8_t channelID ){
	return uch.config.channel_enable[channelID-1];
}

extern void uch_SetChannelConfiguration ( uint8_t channelID , struct uch_config_s cfg ){
	uch.config.channel_config[channelID-1] = cfg;
}

extern void uch_SetChannelState ( uint8_t channelID , bool state ){
	uch.config.channel_enable[channelID-1] = state;
    lw_GPIO_Write(&uch.phy.ch_Enable[channelID-1],state);

}
volatile uint32_t uart_count = 0u;

void USART3_IRQHandler( void ){
	//HAL_UART_IRQHandler(&uch.phy.ch_Uart[0u]);
	uart_count++;
}
void USART2_IRQHandler( void ){
	//HAL_UART_IRQHandler(&uch.phy.ch_Uart[1u]);
}
void USART1_IRQHandler( void ){
	//HAL_UART_IRQHandler(&uch.phy.ch_Uart[2u]);
}

extern void uch_ApplyConfiguration ( uint8_t channelID ){

	if ( channelID > UCH_MAX_NUM || channelID == 0u ){
		Common_Printf("Invalid Channel %d\r\n" , channelID);
		return;
	}
	/* Only Apply Enable Pin Configuration for now */
	if ( !assertConfiguration(channelID)){
		Common_Printf("Bad Configuration for Channel %d\r\n" , channelID);
		return;
	}
	/* Apply UART Configuration */

	uint32_t baudRate = uch.config.channel_config[channelID-1].baudRate;
	uint32_t dataBits = 0U;
	uint32_t stopBits = 0U;
	uint32_t parity = 0U;

	switch(uch.config.channel_config[channelID-1].nDataBits){
		case e_uch_nDatabits_8:
		dataBits = LW_UART_NUM_DATA_BITS_8;
		break;
		case e_uch_nDatabits_9:
		dataBits = LW_UART_NUM_DATA_BITS_9;
		break;
		default:
		break;
	}

	switch(uch.config.channel_config[channelID-1].nStopBits){

		case e_uch_nStopbits_1:
		stopBits = LW_UART_NUM_STOP_BITS_1;
		break;
		case e_uch_nStopbits_2:
		stopBits = LW_UART_NUM_STOP_BITS_2;
		break;
		default:
		break;
	}

	switch(uch.config.channel_config[channelID-1].parity){

		case e_uch_parity_none:
		parity = LW_UART_PARITY_NONE;
		break;
		case e_uch_parity_even:
		parity = LW_UART_PARITY_EVEN;
		break;
		case e_uch_parity_odd:
		parity = LW_UART_PARITY_ODD;
		break;
		default:
		break;
	}

	uch.phy.ch_Uart[channelID-1].data.baudRate = baudRate;
	uch.phy.ch_Uart[channelID-1].data.dataBits = dataBits;
	uch.phy.ch_Uart[channelID-1].data.stopBits = stopBits;
	uch.phy.ch_Uart[channelID-1].data.parity = parity;

	lw_UART_Init(&uch.phy.ch_Uart[channelID-1]);

	lw_Sys_Delay(500);
	/* Apply Channel Enable */
	uch_SetChannelState(channelID,true);

	lw_Sys_Delay(500);

	uint32_t r = uch.phy.ch_Uart[channelID-1].hwctx->DR; // Read any Sporadic Character

	lw_UART_EnableIRQ(&uch.phy.ch_Uart[channelID-1],LW_UART_IRQ_RXNE);

    if ( channelID == CHANNEL_1 ){
    	NVIC_SetPriority(USART3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1U, 1U));
    	NVIC_EnableIRQ(USART3_IRQn);
    }
    else if ( channelID == CHANNEL_2 ){
    	NVIC_SetPriority(USART3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 2U, 2U));
    	NVIC_EnableIRQ(USART2_IRQn);
    }
    else if ( channelID == CHANNEL_3 ){
    	NVIC_SetPriority(USART3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 3U, 3U));
    	NVIC_EnableIRQ(USART1_IRQn);
    }
}

static uint32_t str2int(char* str, uint32_t len)
{
    int i;
    int ret = 0;
    for(i = 0; i < len; ++i)
    {
        ret = ret * 10 + (str[i] - '0');
    }
    return ret;
}

extern void uch_EnumerateChannels( void ){

	uint32_t jsnFileLen = 0u;
	char* jsnFile;

	_fsys_err fsys_err = Fsys_GetConfig(&jsnFile, &jsnFileLen);

	if ( fsys_err != FSYS_SUCCESS ){
		return;
	}

	int tokenCount = jsmn_parse(&uch.json.parser,jsnFile,jsnFileLen,&uch.json.tokens[0u],
			sizeof(uch.json.tokens)/sizeof(uch.json.tokens[0u]));

	for ( int i = 0 ; i < tokenCount ; i++ ){
		if ( uch.json.tokens[i].type == JSMN_OBJECT ){
			struct uch_config_s cfg;
			int32_t channelID = -1;
			cfg.baudRate = UINT32_MAX;
			cfg.nDataBits = UINT8_MAX;
			cfg.nStopBits = UINT8_MAX;
			cfg.parity = UINT8_MAX;

			for ( int j = 0; j < uch.json.tokens[i].size*2u ; j+=2 ){
				uint8_t key = i+j+1u;
				uint32_t key_sz = uch.json.tokens[key].end-uch.json.tokens[key].start;
				uint8_t value = i+j+2u;
				uint32_t value_sz = uch.json.tokens[value].end - uch.json.tokens[value].start;
				uint32_t kValue = UINT32_MAX;
				if ( !strncmp(&jsnFile[uch.json.tokens[key].start],"channelId",key_sz) ){
					int32_t kValue = jsnFile[uch.json.tokens[value].start]- '0';
					channelID = kValue;
				}
				else if (!strncmp(&jsnFile[uch.json.tokens[key].start],"baudrate",
					(unsigned int)(uch.json.tokens[key].end-uch.json.tokens[key].start))){
					kValue = str2int( &jsnFile[uch.json.tokens[value].start], value_sz);
					cfg.baudRate = kValue;
				}
				else if (!strncmp(&jsnFile[uch.json.tokens[key].start],"parity",key_sz)){

					if(!strncmp(&jsnFile[uch.json.tokens[value].start],"even",value_sz)){
						kValue = e_uch_parity_even;
					}
					else if(!strncmp(&jsnFile[uch.json.tokens[value].start],"odd",value_sz)){
						kValue = e_uch_parity_odd;
					}
					else if(!strncmp(&jsnFile[uch.json.tokens[value].start],"none",value_sz)){
						kValue = e_uch_parity_none;
					}
					cfg.parity = kValue;
				}
				else if( !strncmp(&jsnFile[uch.json.tokens[key].start],"numberOfDataBits",key_sz)){
					kValue = str2int( &jsnFile[uch.json.tokens[value].start], value_sz);
					if ( kValue == 8u ){
						cfg.nDataBits = e_uch_nDatabits_8;
					}
					else if ( kValue == 9u ){
						cfg.nDataBits = e_uch_nDatabits_9;
					}
				}
				else if (!strncmp(&jsnFile[uch.json.tokens[key].start],"numberOfStopBits",key_sz)){
					kValue = str2int( &jsnFile[uch.json.tokens[value].start],value_sz);
					if ( kValue == 1u ){
						cfg.nStopBits = e_uch_nStopbits_1;
					}
					else if ( kValue == 2u ){
						cfg.nStopBits = e_uch_nStopbits_2;
					}
				}
				else{

				}
			}
			/* Copy configuration of valid channels */
			if ( channelID >= 1u || channelID <= UCH_MAX_NUM ){
				uch.config.channel_config[channelID-1u] = cfg;
			}
		}
		else{
			continue;
		}
	}

}
