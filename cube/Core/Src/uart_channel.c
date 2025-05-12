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
#include <string.h>

#define UCH_MAX_NUM 3u
#define UCH_CONFIG_MAX_TOKEN_SIZE 64u


struct {

	struct {
		GPIO_HandleTypeDef gpioH[UCH_MAX_NUM];
		UART_HandleTypeDef uartH[UCH_MAX_NUM];
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

static uint8_t arr[512];
static void uch_phyInit( void ){

	uch.phy.uartH[0u].Instance = USART3;
	uch.phy.uartH[0u].Init.BaudRate = 115200;
	uch.phy.uartH[0u].Init.WordLength = UART_WORDLENGTH_8B;
	uch.phy.uartH[0u].Init.StopBits = UART_STOPBITS_1;
	uch.phy.uartH[0u].Init.Parity = UART_PARITY_NONE;
	uch.phy.uartH[0u].Init.Mode = UART_MODE_TX_RX;
	uch.phy.uartH[0u].Init.HwFlowCtl = UART_HWCONTROL_NONE;
	uch.phy.uartH[0u].Init.OverSampling = UART_OVERSAMPLING_16;
	uch.phy.uartH[0u].pRxBuffPtr = &arr[0];
	HAL_UART_Init(&uch.phy.uartH[0u]);

	uch.phy.gpioH[0u].pinData.Pin = GPIO_PIN_0;
	uch.phy.gpioH[0u].pinData.Mode = GPIO_MODE_OUTPUT_PP;
	uch.phy.gpioH[0u].pinData.Pull = GPIO_NOPULL;
	uch.phy.gpioH[0u].pinData.Speed = GPIO_SPEED_FREQ_LOW;
	uch.phy.gpioH[0u].port = GPIOB;
	uch.phy.gpioH[0u].pinNum = 0u;

    HAL_GPIO_Init(uch.phy.gpioH[0u].port, &uch.phy.gpioH[0u].pinData);
    HAL_GPIO_WritePin(uch.phy.gpioH[0u].port, uch.phy.gpioH[0u].pinData.Pin, _INIT_DEBUG_EN);


	uch.phy.uartH[1u].Instance = USART2;
	uch.phy.uartH[1u].Init.BaudRate = 115200;
	uch.phy.uartH[1u].Init.WordLength = UART_WORDLENGTH_8B;
	uch.phy.uartH[1u].Init.StopBits = UART_STOPBITS_1;
	uch.phy.uartH[1u].Init.Parity = UART_PARITY_NONE;
	uch.phy.uartH[1u].Init.Mode = UART_MODE_TX_RX;
	uch.phy.uartH[1u].Init.HwFlowCtl = UART_HWCONTROL_NONE;
	uch.phy.uartH[1u].Init.OverSampling = UART_OVERSAMPLING_16;

	HAL_UART_Init(&uch.phy.uartH[1u]);

	uch.phy.gpioH[1u].pinData.Pin = GPIO_PIN_1;
	uch.phy.gpioH[1u].pinData.Mode = GPIO_MODE_OUTPUT_PP;
	uch.phy.gpioH[1u].pinData.Pull = GPIO_NOPULL;
	uch.phy.gpioH[1u].pinData.Speed = GPIO_SPEED_FREQ_LOW;
	uch.phy.gpioH[1u].port = GPIOB;
	uch.phy.gpioH[1u].pinNum = 1u;

    HAL_GPIO_Init(uch.phy.gpioH[1u].port, &uch.phy.gpioH[1u].pinData);
    HAL_GPIO_WritePin(uch.phy.gpioH[1u].port, uch.phy.gpioH[1u].pinData.Pin, GPIO_PIN_RESET);

	uch.phy.uartH[2u].Instance = USART1;
	uch.phy.uartH[2u].Init.BaudRate = 115200;
	uch.phy.uartH[2u].Init.WordLength = UART_WORDLENGTH_8B;
	uch.phy.uartH[2u].Init.StopBits = UART_STOPBITS_1;
	uch.phy.uartH[2u].Init.Parity = UART_PARITY_NONE;
	uch.phy.uartH[2u].Init.Mode = UART_MODE_TX_RX;
	uch.phy.uartH[2u].Init.HwFlowCtl = UART_HWCONTROL_NONE;
	uch.phy.uartH[2u].Init.OverSampling = UART_OVERSAMPLING_16;

	HAL_UART_Init(&uch.phy.uartH[2u]);

	uch.phy.gpioH[2u].pinData.Pin = GPIO_PIN_2;
	uch.phy.gpioH[2u].pinData.Mode = GPIO_MODE_OUTPUT_PP;
	uch.phy.gpioH[2u].pinData.Pull = GPIO_NOPULL;
	uch.phy.gpioH[2u].pinData.Speed = GPIO_SPEED_FREQ_LOW;
	uch.phy.gpioH[2u].port = GPIOB;
	uch.phy.gpioH[2u].pinNum = 2u;

    HAL_GPIO_Init(uch.phy.gpioH[2u].port, &uch.phy.gpioH[2u].pinData);
    HAL_GPIO_WritePin(uch.phy.gpioH[2u].port, uch.phy.gpioH[2u].pinData.Pin, GPIO_PIN_RESET);

}
extern void uch_Init ( void ){

	for ( uint8_t i = 0 ; i < UCH_MAX_NUM ; i++ ){
		uch.config.channel_enable[i] = false;
		uch.config.channel_config[i].baudRate = UINT32_MAX;
		uch.config.channel_config[i].nDataBits = UINT8_MAX;
		uch.config.channel_config[i].nStopBits = UINT8_MAX;
		uch.config.channel_config[i].parity = UINT8_MAX;
	}

	/* Init Channel Peripherals */
	uch_phyInit();

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
    HAL_GPIO_WritePin(uch.phy.gpioH[channelID-1].port, uch.phy.gpioH[channelID-1].pinData.Pin,
    		state == true ? GPIO_PIN_SET : GPIO_PIN_RESET);

}
volatile uint32_t uart_count = 0u;

void USART3_IRQHandler( void ){
	HAL_UART_IRQHandler(&uch.phy.uartH[0u]);
	uart_count++;
}
void USART2_IRQHandler( void ){
	HAL_UART_IRQHandler(&uch.phy.uartH[1u]);
}
void USART1_IRQHandler( void ){
	HAL_UART_IRQHandler(&uch.phy.uartH[2u]);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	Common_Printf("rx\r\n");
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
		dataBits = UART_WORDLENGTH_8B;
		break;
		case e_uch_nDatabits_9:
		dataBits = UART_WORDLENGTH_9B;
		break;
		default:
		break;
	}

	switch(uch.config.channel_config[channelID-1].nStopBits){

		case e_uch_nStopbits_1:
		stopBits = UART_STOPBITS_1;
		break;
		case e_uch_nStopbits_2:
		stopBits = UART_STOPBITS_2;
		break;
		default:
		break;
	}

	switch(uch.config.channel_config[channelID-1].parity){

		case e_uch_parity_none:
		parity = UART_PARITY_NONE;
		break;
		case e_uch_parity_even:
		parity = UART_PARITY_EVEN;
		break;
		case e_uch_parity_odd:
		parity = UART_PARITY_ODD;
		break;
		default:
		break;
	}

	uch.phy.uartH[channelID-1].Init.BaudRate = baudRate;
	uch.phy.uartH[channelID-1].Init.WordLength = dataBits;
	uch.phy.uartH[channelID-1].Init.StopBits = stopBits;
	uch.phy.uartH[channelID-1].Init.Parity = parity;

	HAL_UART_Init(&uch.phy.uartH[channelID-1]);

    HAL_Delay(500);
	/* Apply Channel Enable */
	uch_SetChannelState(channelID,true);

	HAL_Delay(500);

	uint32_t r = uch.phy.uartH[channelID-1].Instance->DR; // Read any Sporadic Character
	UNUSED(r);

	__HAL_UART_ENABLE_IT(&uch.phy.uartH[channelID-1],UART_IT_RXNE);

    if ( channelID == CHANNEL_1 ){
    	HAL_NVIC_SetPriority(USART3_IRQn, 1, 1);
    	HAL_NVIC_EnableIRQ(USART3_IRQn);
    	UART_Start_Receive_IT(&uch.phy.uartH[0u], arr, 10);

    }
    else if ( channelID == CHANNEL_2 ){
    	HAL_NVIC_SetPriority(USART2_IRQn, 2, 2);
    	HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
    else if ( channelID == CHANNEL_3 ){
    	HAL_NVIC_SetPriority(USART1_IRQn, 3, 3);
    	HAL_NVIC_EnableIRQ(USART1_IRQn);
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
