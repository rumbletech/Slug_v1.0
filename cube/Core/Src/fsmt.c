/*
 * fsmt.c
 *
 *  Created on: Jun 21, 2025
 *      Author: rumbl
 */


#include "fsmt.h"
#include "com_channel.h"
#include "rgb.h"
#include "diskt.h"
#include "jsmn.h"
#include <string.h>

enum e_fsm_state {

	e_fsm_state_init,
	e_fsm_state_enumerate_hardware,
	e_fsm_state_load_configuration,
	e_fsm_state_apply_configuration,
	e_fsm_state_monitor,
	e_fsm_state_error,
	e_fsm_state_LENGTH,

};

enum e_fsm_err {

	e_fsm_err_ok,
	e_fsm_err_mount_failed,
	e_fsm_err_cfg_read_failed,
	e_fsm_err_no_active_comm,
	e_fsm_err_LENGTH,
};

#define CONFIG_FILE_READ_SZ 512U
#define FSM_RGB_DURATION_MS 125U

#define JSON_TOK_MAX_SIZE 128U

static enum e_fsm_state fsm_cs = e_fsm_state_init;
static enum e_fsm_state fsm_ns = e_fsm_state_init;
static enum e_fsm_state fsm_ps = e_fsm_state_init;
static bool fsm_ini = false;
static uint32_t fsm_cnt = 0u;
static enum e_fsm_err fsm_err = 0u;

static uint8_t fsm_state_clt[e_fsm_state_LENGTH] = { 125U,125U,125U,125U,125U,125U};
static uint8_t fsm_state_c1[e_fsm_state_LENGTH] = { COLOR_GREEN,COLOR_GREEN,COLOR_GREEN,COLOR_GREEN,COLOR_MAGNETA,COLOR_RED};
static uint8_t fsm_state_c2[e_fsm_state_LENGTH] = { COLOR_BLUE,COLOR_BLUE,COLOR_BLUE,COLOR_BLUE,COLOR_MAGNETA,COLOR_BLACK };


static lw_uart_data_t com_channels_cfg[COM_CHANNEL_LENGTH];
static bool com_channels_cfg_valid[COM_CHANNEL_LENGTH];

jsmntok_t json_tokens[JSON_TOK_MAX_SIZE];
static jsmn_parser json_parser;



#if _USE_LFN != 0
static char configJsonFilePath[] = "0:/slug/config.json";
#else
static char configJsonFilePath[] = "0:/slug/CONFIG~1.JSO";
#endif
static FIL configJsonFile;
static uint8_t configJsonBuffer[CONFIG_FILE_READ_SZ];
//static uint32_t configJsonFileLength;




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

extern void Enumerate_COM_Channels( void ){

	uint32_t jsnFileLen = strlen((const char*)configJsonBuffer);
	char* jsnFile = (char*)&configJsonBuffer[0u];

	int tokenCount = jsmn_parse(&json_parser,jsnFile,jsnFileLen,&json_tokens[0U],
			sizeof(json_tokens)/sizeof(json_tokens[0u]));

	for ( int i = 0 ; i < tokenCount ; i++ ){
		if ( json_tokens[i].type == JSMN_OBJECT ){
			lw_uart_data_t cfg;
			int32_t channelID = -1;
			cfg.baudRate = UINT32_MAX;
			cfg.dataBits = UINT8_MAX;
			cfg.stopBits = UINT8_MAX;
			cfg.parity = UINT8_MAX;

			for ( int j = 0; j < json_tokens[i].size*2u ; j+=2 ){
				uint8_t key = i+j+1u;
				uint8_t value = i+j+2u;
				unsigned int key_sz = json_tokens[key].end-json_tokens[key].start;
				unsigned int value_sz = json_tokens[value].end - json_tokens[value].start;
				uint32_t kValue = UINT32_MAX;
				if ( !strncmp(&jsnFile[json_tokens[key].start],"channelId",key_sz) ){
					int32_t kValue = jsnFile[json_tokens[value].start]- '0';
					channelID = kValue;
				}
				else if (!strncmp(&jsnFile[json_tokens[key].start],"baudrate",
					(unsigned int)(json_tokens[key].end-json_tokens[key].start))){
					kValue = str2int( &jsnFile[json_tokens[value].start], value_sz);
					cfg.baudRate = kValue;
				}
				else if (!strncmp(&jsnFile[json_tokens[key].start],"parity",key_sz)){

					if(!strncmp(&jsnFile[json_tokens[value].start],"even",value_sz)){
						kValue = LW_UART_PARITY_EVEN;
					}
					else if(!strncmp(&jsnFile[json_tokens[value].start],"odd",value_sz)){
						kValue = LW_UART_PARITY_ODD;
					}
					else if(!strncmp(&jsnFile[json_tokens[value].start],"none",value_sz)){
						kValue = LW_UART_PARITY_NONE;
					}
					cfg.parity = kValue;
				}
				else if( !strncmp(&jsnFile[json_tokens[key].start],"numberOfDataBits",key_sz)){
					kValue = str2int( &jsnFile[json_tokens[value].start], value_sz);
					if ( kValue == 8u ){
						cfg.dataBits = LW_UART_NUM_DATA_BITS_8;
					}
					else if ( kValue == 9u ){
						cfg.dataBits = LW_UART_NUM_DATA_BITS_9;
					}
				}
				else if (!strncmp(&jsnFile[json_tokens[key].start],"numberOfStopBits",key_sz)){
					kValue = str2int( &jsnFile[json_tokens[value].start],value_sz);
					if ( kValue == 1u ){
						cfg.stopBits = LW_UART_NUM_STOP_BITS_1;
					}
					else if ( kValue == 2u ){
						cfg.stopBits = LW_UART_NUM_STOP_BITS_2;
					}
				}
				else{

				}
			}
			/* Copy configuration of valid channels */
			if ( channelID >= 1u || channelID <= 3u ){
				com_channels_cfg[channelID-1u] = cfg;
				com_channels_cfg_valid[channelID-1u] = true;
			}
		}
		else{
			continue;
		}
	}

}

static ProcStatus_t ReadConfig_process( void ){

	static uint32_t stepCnt = 0u;
	static uint32_t stepReturn = E_PENDING;

	struct diskt_request_s diskRequest;
	struct diskt_response_s diskResponse;

	static struct  diskt_request_open_s open_req = {
			.fptr = &configJsonFile ,
			.path = &configJsonFilePath[0u] ,
			.opts = FA_READ
	};

	static struct  diskt_request_read_s read_req = {
			.fptr = &configJsonFile ,
			.readBuffer = &configJsonBuffer[0u] ,
			.maxSize = sizeof(configJsonBuffer)
	};

	static struct  diskt_request_close_s close_req = {
			.fptr = &configJsonFile
	};


	if ( fsm_ini == true ){
		stepCnt = 0u;
	}


	if ( stepCnt == 0u ){
		diskRequest.reqType = e_diskt_open_file;
		diskRequest.args = (void*)&open_req;
		stepReturn = DISKT_PostRequest(diskRequest);
	}

	if ( stepCnt == 1u ){
		stepReturn = DISKT_GetResponse(&diskResponse);
		if ( stepReturn == E_OK  ){
			stepReturn = diskResponse.proc_rc;
		}
	}

	if ( stepCnt == 2u ){
		diskRequest.reqType = e_diskt_read_file;
		diskRequest.args = (void*)&read_req;
		stepReturn = DISKT_PostRequest(diskRequest);
	}

	if ( stepCnt == 3u ){
		stepReturn = DISKT_GetResponse(&diskResponse);
		if ( stepReturn == E_OK  ){
			stepReturn = diskResponse.proc_rc;
		}
	}

	if ( stepCnt == 4u ){
		diskRequest.reqType = e_diskt_close_file;
		diskRequest.args = (void*)&close_req;
		stepReturn = DISKT_PostRequest(diskRequest);
	}

	if ( stepCnt == 5u ){
		stepReturn = DISKT_GetResponse(&diskResponse);
		if ( stepReturn == E_OK  ){
			stepReturn = diskResponse.proc_rc;
		}
	}

	if ( stepReturn == E_OK ){
		stepCnt++;
		if ( stepCnt == 6u ){
			return E_OK;
		}
		stepReturn = E_PENDING;
	}

	return stepReturn;

}

static ProcStatus_t mountSDC_process( void ){

	static uint32_t stepCnt = 0u;
	static uint32_t stepReturn = E_PENDING;

	struct diskt_request_s diskRequest;
	struct diskt_response_s diskResponse;


	if ( fsm_ini == true ){
		stepCnt = 0u;
	}


	if ( stepCnt == 0u ){
		diskRequest.reqType = e_diskt_request_mount;
		stepReturn = DISKT_PostRequest(diskRequest);
	}

	if ( stepCnt == 1u ){
		stepReturn = DISKT_GetResponse(&diskResponse);
		if ( stepReturn == E_OK  ){
			stepReturn = diskResponse.proc_rc;
		}
	}

	if ( stepCnt == 2u ){
		diskRequest.reqType = e_diskt_request_get_label;
		stepReturn = DISKT_PostRequest(diskRequest);
	}

	if ( stepCnt == 3u ){
		stepReturn = DISKT_GetResponse(&diskResponse);
	}

	if ( stepReturn == E_OK ){
		stepCnt++;
		if ( stepCnt == 4u ){
			return E_OK;
		}
		stepReturn = E_PENDING;
	}

	return stepReturn;
}


static void fsm_init_h ( void ){

	Common_Printf("FSM_STATE_INIT\r\n");

	for ( uint8_t i = 0 ; i < COM_CHANNEL_LENGTH ; i++ ){
		com_channels_cfg_valid[i] = false;
		com_channels_cfg[i].baudRate = UINT32_MAX;
		com_channels_cfg[i].dataBits = UINT8_MAX;
		com_channels_cfg[i].stopBits = UINT8_MAX;
		com_channels_cfg[i].parity = UINT8_MAX;
#if defined(_OPTS_DEBUG_EN) && _OPTS_DEBUG_EN == true
		if ( i == COM_CHANNEL_1 ){
			continue;
		}
#endif
		Com_Channel_StopLogging(i);
		Com_Channel_Disable(i);

	}

	fsm_ns = e_fsm_state_enumerate_hardware;
	fsm_err = e_fsm_err_ok;

}

static void fsm_enumerate_hardware_h( void ) {

	Common_Printf("FSM_STATE_ENUM_HW\r\n");

	ProcStatus_t ret = mountSDC_process();

	if ( ret == E_OK ){
		fsm_ns = e_fsm_state_load_configuration;
		fsm_err = e_fsm_err_ok;
	}
	else if ( ret == E_PENDING ){
		fsm_ns = e_fsm_state_enumerate_hardware;
		fsm_err = e_fsm_err_ok;
	}
	else{
		fsm_ns = e_fsm_state_error;
		fsm_err = e_fsm_err_mount_failed;

	}

}

static void fsm_load_configuration_h( void ){

	Common_Printf("FSM_STATE_LOAD_CFG\r\n");

	ProcStatus_t ret = 	ReadConfig_process();

	if ( ret == E_OK ){
		fsm_ns = e_fsm_state_apply_configuration;
		fsm_err = e_fsm_err_ok;
	}
	else if ( ret == E_PENDING ){
		fsm_ns = e_fsm_state_load_configuration;
		fsm_err = e_fsm_err_ok;
	}
	else{
		fsm_ns = e_fsm_state_error;
		fsm_err = e_fsm_err_cfg_read_failed;

	}
}


static void fsm_apply_configuration_h( void ) {

	Common_Printf("FSM_STATE_APPLY_CFG\r\n");

	Enumerate_COM_Channels();

	bool isActive = false;

	for ( uint8_t i = 0u ; i < COM_CHANNEL_LENGTH ; i++ ){
		isActive = (isActive || com_channels_cfg_valid[i]);
		if ( com_channels_cfg_valid[i] == true ){
			  Com_Channel_Configure(i, com_channels_cfg[i]);
			  Com_Channel_Enable(i);
		}
		else{
			  Com_Channel_StopLogging(i);
			  Com_Channel_Disable(i);
		}
	}

	if ( isActive ){
		fsm_ns = e_fsm_state_monitor;
		fsm_err = e_fsm_err_ok;
	}
	else{
		fsm_ns = e_fsm_state_error;
		fsm_err = e_fsm_err_no_active_comm;
	}
}

static void fsm_monitor_h( void ) {

	//Common_Printf("FSM_STATE_MONITOR\r\n");

	fsm_ns = e_fsm_state_monitor;
	fsm_err = e_fsm_err_ok;

}

static void fsm_default_h ( void ){


	Common_Printf("FSM_STATE_UNKNOWN\r\n");
}


static void fsm_illuminate_state( void ){

	if ( fsm_cnt%(2u*(fsm_state_clt[fsm_cs]/FSM_CYCLE_DURATION_MS)) == 0U ){
		RGB_Write(fsm_state_c1[fsm_cs]);
	}
	else if ( fsm_cnt%(1u*(fsm_state_clt[fsm_cs]/FSM_CYCLE_DURATION_MS)) == 0U ){
		RGB_Write(fsm_state_c2[fsm_cs]);
	}

}

static void fsm_err_h ( void ){
	Common_Printf("FSM_STATE_ERR\r\n");

	/* Handle Errors */
	switch( fsm_err ){
	default:
		break;
	}

	fsm_ns = e_fsm_state_error;

}

extern void fsm_init( void ){
	jsmn_init(&json_parser); /* Initialize jsmn jason parser */

	fsm_cs = e_fsm_state_init;
	fsm_ns = e_fsm_state_init;
	fsm_ps = e_fsm_state_init;
	fsm_ini = false;
	fsm_cnt = 0u;
	fsm_err = e_fsm_err_ok;
}

extern void fsm ( void ){

	fsm_ps = fsm_cs;
	fsm_cs = fsm_ns ;

	fsm_ini =  fsm_cs != fsm_ps;

	fsm_illuminate_state();

	switch(fsm_cs){

	case e_fsm_state_init:
		fsm_init_h();
	break;
	case e_fsm_state_enumerate_hardware:
		fsm_enumerate_hardware_h();
	break;
	case e_fsm_state_load_configuration:
		fsm_load_configuration_h();
	break;
	case e_fsm_state_apply_configuration:
		fsm_apply_configuration_h();
	break;
	case e_fsm_state_monitor:
		fsm_monitor_h();
	break;
	case e_fsm_state_error:
		fsm_err_h();
	break;
	default:
		fsm_default_h();
	break;
	}

	fsm_cnt++;


}
