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
	e_fsm_state_enum_configuration,
	e_fsm_state_open_log_files,
	e_fsm_state_apply_configuration,
	e_fsm_state_monitor,
	e_fsm_state_error,
	e_fsm_state_LENGTH,

};

enum e_err {

	e_err_ok,
	e_err_mount_failed,
	e_err_cfg_read_failed,
	e_err_open_logs_failed,
	e_err_no_active_comm,
	e_err_LENGTH,
};

struct fsm_state_s {
	uint8_t isToggle;
	uint8_t togglePeriod;
	uint8_t colorCycle1;
	uint8_t colorCycle2;

};

#define CONFIG_FILE_READ_SZ 512U
#define FSM_RGB_DURATION_MS 125U
#define JSON_TOK_MAX_SIZE 128U
#define COM_CHANNEL_POLL_PERIOD_MS 100u
#define COM_CHANNEL_FLUSH_PERIOD_MS 1000U

static struct {

	 enum e_fsm_state cs;
	 enum e_fsm_state ns;
	 enum e_fsm_state ps;
	 bool ini;
	 uint32_t cnt;
	 enum e_err err;
}fsmt;


static struct fsm_state_s fsm_states[e_fsm_state_LENGTH] = {

		[e_fsm_state_init] = {
				.isToggle = 1U,
				.togglePeriod = 125U,
				.colorCycle1 = COLOR_GREEN,
				.colorCycle2 = COLOR_BLUE
		},
		[e_fsm_state_enumerate_hardware] = {
				.isToggle = 1U,
				.togglePeriod = 125U,
				.colorCycle1 = COLOR_GREEN,
				.colorCycle2 = COLOR_BLUE
		},
		[e_fsm_state_load_configuration] = {
				.isToggle = 1U,
				.togglePeriod = 125U,
				.colorCycle1 = COLOR_GREEN,
				.colorCycle2 = COLOR_BLUE
		},
		[e_fsm_state_enum_configuration] = {
				.isToggle = 1U,
				.togglePeriod = 125U,
				.colorCycle1 = COLOR_GREEN,
				.colorCycle2 = COLOR_BLUE
		},
		[e_fsm_state_open_log_files] = {
				.isToggle = 1U,
				.togglePeriod = 125U,
				.colorCycle1 = COLOR_GREEN,
				.colorCycle2 = COLOR_BLUE
		},
		[e_fsm_state_apply_configuration] = {
				.isToggle = 1U,
				.togglePeriod = 125U,
				.colorCycle1 = COLOR_GREEN,
				.colorCycle2 = COLOR_BLUE
		},
		[e_fsm_state_monitor] = {
				.isToggle = 0U,
				.togglePeriod = 125U,
				.colorCycle1 = COLOR_MAGNETA,
				.colorCycle2 = COLOR_MAGNETA
		},
		[e_fsm_state_error] = {
				.isToggle = 1U,
				.togglePeriod = 125U,
				.colorCycle1 = COLOR_RED,
				.colorCycle2 = COLOR_BLACK
		}


};

static lw_uart_data_t com_channels_cfg[COM_CHANNEL_LENGTH];
static bool com_channels_cfg_valid[COM_CHANNEL_LENGTH];
static bool com_channels_valid;

jsmntok_t json_tokens[JSON_TOK_MAX_SIZE];
static jsmn_parser json_parser;

#if _USE_LFN != 0
static char configJsonFilePath[] = "0:/slug/config.json";
#else
static char configJsonFilePath[] = "0:/slug/CONFIG~1.JSO";
#endif
static FIL configJsonFile;
static uint8_t configJsonBuffer[CONFIG_FILE_READ_SZ];

static char* logFilesPath[COM_CHANNEL_LENGTH] = {
#if _USE_LFN != 0
"0:/slug/logs/log_ch1.txt"
#else
"0:/slug/logs/LOG_CH1~1.TXT"
#endif
,
#if _USE_LFN != 0
"0:/slug/logs/log_ch2.txt"
#else
"0:/slug/logs/LOG_CH1~2.TXT"
#endif
,
#if _USE_LFN != 0
"0:/slug/logs/log_ch3.txt"
#else
"0:/slug/logs/LOG_CH1~3.TXT"
#endif
};
static FIL logFiles[COM_CHANNEL_LENGTH];


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

	if ( fsmt.ini == true ){
		stepCnt = 0u;
	}


	if ( stepCnt == 0u ){
		diskRequest.reqArg.open_request.fptr = &configJsonFile;
		diskRequest.reqArg.open_request.path = &configJsonFilePath[0u];
		diskRequest.reqArg.open_request.opts = FA_READ;
		diskRequest.reqType = e_diskt_open_file;
		stepReturn = DISKT_PostRequest(diskRequest);
	}

	if ( stepCnt == 1u ){
		stepReturn = DISKT_GetResponse(&diskResponse);
		if ( stepReturn == E_OK  ){
			stepReturn = diskResponse.proc_rc;
		}
	}

	if ( stepCnt == 2u ){
		diskRequest.reqArg.read_request.fptr = &configJsonFile;
		diskRequest.reqArg.read_request.readBuffer = &configJsonBuffer[0u];
		diskRequest.reqArg.read_request.maxSize = sizeof(configJsonBuffer);
		diskRequest.reqType = e_diskt_read_file;
		stepReturn = DISKT_PostRequest(diskRequest);
	}

	if ( stepCnt == 3u ){
		stepReturn = DISKT_GetResponse(&diskResponse);
		if ( stepReturn == E_OK  ){
			stepReturn = diskResponse.proc_rc;
		}
	}

	if ( stepCnt == 4u ){
		diskRequest.reqArg.close_request.fptr = &configJsonFile;
		diskRequest.reqType = e_diskt_close_file;
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

static ProcStatus_t OpenLogFiles_process( void ){

	static uint32_t stepCnt = 0u;
	static ProcStatus_t stepReturn = E_PENDING;

	struct diskt_request_s diskRequest;
	struct diskt_response_s diskResponse;

	if ( fsmt.ini == true ){
		stepCnt = 0u;
	}

	if ( stepCnt == 0u  ){
		if ( com_channels_cfg_valid[COM_CHANNEL_1]) {
			Common_Printf("Opening File : %s\r\n" , &logFilesPath[COM_CHANNEL_1][0]);
			diskRequest.reqArg.open_request.fptr = &logFiles[COM_CHANNEL_1];
			diskRequest.reqArg.open_request.path = &logFilesPath[COM_CHANNEL_1][0];
			diskRequest.reqArg.open_request.opts = FA_CREATE_ALWAYS|FA_OPEN_ALWAYS|FA_WRITE;
			diskRequest.reqType = e_diskt_open_file;
			stepReturn = DISKT_PostRequest(diskRequest);
		}
		else{
			stepReturn = E_OK;
		}
	}

	if ( stepCnt == 1u ){
		if ( com_channels_cfg_valid[COM_CHANNEL_1]){
			stepReturn = DISKT_GetResponse(&diskResponse);
			if ( stepReturn == E_OK  ){
				stepReturn = diskResponse.proc_rc;
			}
		}
		else{
			stepReturn = E_OK;
		}
	}

	if ( stepCnt == 2u  ){
		if ( com_channels_cfg_valid[COM_CHANNEL_2] ){
			diskRequest.reqArg.open_request.fptr = &logFiles[COM_CHANNEL_2];
			diskRequest.reqArg.open_request.path = &logFilesPath[COM_CHANNEL_2][0];
			diskRequest.reqArg.open_request.opts = FA_CREATE_ALWAYS|FA_OPEN_ALWAYS|FA_WRITE;
			diskRequest.reqType = e_diskt_open_file;
			stepReturn = DISKT_PostRequest(diskRequest);
		}
		else{
			stepReturn = E_OK;
		}
	}

	if ( stepCnt == 3u  ){
		if ( com_channels_cfg_valid[COM_CHANNEL_2] ){
			stepReturn = DISKT_GetResponse(&diskResponse);
			if ( stepReturn == E_OK  ){
				stepReturn = diskResponse.proc_rc;
			}
		}
		else{
			stepReturn = E_OK;
		}
	}

	if ( stepCnt == 4u ){
		if ( com_channels_cfg_valid[COM_CHANNEL_3] ){
			diskRequest.reqArg.open_request.fptr = &logFiles[COM_CHANNEL_3];
			diskRequest.reqArg.open_request.path = &logFilesPath[COM_CHANNEL_3][0];
			diskRequest.reqArg.open_request.opts = FA_CREATE_ALWAYS|FA_OPEN_ALWAYS|FA_WRITE;
			diskRequest.reqType = e_diskt_open_file;
			stepReturn = DISKT_PostRequest(diskRequest);
		}
		else{
			stepReturn = E_OK;
		}
	}

	if ( stepCnt == 5u  ){
		if ( com_channels_cfg_valid[COM_CHANNEL_3] ){
			stepReturn = DISKT_GetResponse(&diskResponse);
			if ( stepReturn == E_OK  ){
				stepReturn = diskResponse.proc_rc;
			}
		}
		else{
			stepReturn = E_OK;
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


	if ( fsmt.ini == true ){
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

	fsmt.ns = e_fsm_state_enumerate_hardware;
	fsmt.err = e_err_ok;

}

static void fsm_enumerate_hardware_h( void ) {

	Common_Printf("FSM_STATE_ENUM_HW\r\n");

	ProcStatus_t ret = mountSDC_process();

	if ( ret == E_OK ){
		fsmt.ns = e_fsm_state_load_configuration;
		fsmt.err = e_err_ok;
	}
	else if ( ret == E_PENDING ){
		fsmt.ns = e_fsm_state_enumerate_hardware;
		fsmt.err = e_err_ok;
	}
	else{
		fsmt.ns = e_fsm_state_error;
		fsmt.err = e_err_mount_failed;

	}

}

static void fsm_load_configuration_h( void ){

	Common_Printf("FSM_STATE_LOAD_CFG\r\n");

	ProcStatus_t ret = 	ReadConfig_process();

	if ( ret == E_OK ){
		fsmt.ns = e_fsm_state_enum_configuration;
		fsmt.err = e_err_ok;
	}
	else if ( ret == E_PENDING ){
		fsmt.ns = e_fsm_state_load_configuration;
		fsmt.err = e_err_ok;
	}
	else{
		fsmt.ns = e_fsm_state_error;
		fsmt.err = e_err_cfg_read_failed;
	}
}

static void fsm_enum_configuration_h( void ){
	Common_Printf("FSM_STATE_ENUM_CFG\r\n");

	Enumerate_COM_Channels();

	for ( uint8_t i = 0 ; i < COM_CHANNEL_LENGTH ; i++ ){

		if ( com_channels_cfg[i].dataBits != LW_UART_NUM_DATA_BITS_8 &&
			 com_channels_cfg[i].dataBits != LW_UART_NUM_DATA_BITS_9 ){
			com_channels_cfg_valid[i] = 0u;
		}

		if ( com_channels_cfg[i].stopBits != LW_UART_NUM_STOP_BITS_1 &&
			 com_channels_cfg[i].stopBits != LW_UART_NUM_STOP_BITS_2 ){
			com_channels_cfg_valid[i] = 0u;
		}

		if ( com_channels_cfg[i].parity != LW_UART_PARITY_NONE &&
			 com_channels_cfg[i].parity != LW_UART_PARITY_EVEN &&
			 com_channels_cfg[i].parity != LW_UART_PARITY_ODD ){
			com_channels_cfg_valid[i] = 0u;
		}

		if ( com_channels_cfg_valid[i] == 0u ){
			  Com_Channel_StopLogging(i);
			  Com_Channel_Disable(i);
		}

		com_channels_valid = com_channels_valid || com_channels_cfg_valid[i];
	}


	if ( com_channels_valid ){
		fsmt.ns = e_fsm_state_open_log_files;
		fsmt.err = e_err_ok;
	}
	else{
		fsmt.ns = e_fsm_state_error;
		fsmt.err = e_err_no_active_comm;
	}
}

static void fsm_open_logs_h ( void ){

	Common_Printf("FSM_STATE_OPEN_LOGGINGG\r\n");

	ProcStatus_t ret = 	OpenLogFiles_process();

	if ( ret == E_OK ){
		fsmt.ns = e_fsm_state_apply_configuration;
		fsmt.err = e_err_ok;
	}
	else if ( ret == E_PENDING ){
		fsmt.ns = e_fsm_state_open_log_files;
		fsmt.err = e_err_ok;
	}
	else{
		fsmt.ns = e_fsm_state_error;
		fsmt.err = e_err_open_logs_failed;
	}

}

static void fsm_apply_configuration_h( void ) {

	Common_Printf("FSM_STATE_APPLY_CFG\r\n");

	/* Configure and Enable Channels */
	for ( uint8_t i = 0u ; i < COM_CHANNEL_LENGTH ; i++ ){
		if ( com_channels_cfg_valid[i] == true ){
			  Com_Channel_Configure(i, com_channels_cfg[i]);
			  Com_Channel_StartLogging(i);
			  Com_Channel_Enable(i);
		}
	}

	if ( com_channels_valid ){
		fsmt.ns = e_fsm_state_monitor;
		fsmt.err = e_err_ok;
	}
	else{
		fsmt.ns = e_fsm_state_error;
		fsmt.err = e_err_no_active_comm;
	}
}

static void fsm_monitor_h( void ) {

	Common_Printf("FSM_STATE_MONITOR\r\n");

	struct diskt_request_s diskRequest;
	struct diskt_response_s diskResponse;


	DISKT_GetResponse(&diskResponse);

	/* Write to Pipe  */
	if ( fsmt.cnt%(COM_CHANNEL_POLL_PERIOD_MS/FSM_CYCLE_DURATION_MS) == 0u ){

		for ( uint8_t i = 0 ; i < COM_CHANNEL_LENGTH ; i++ ){
			if ( com_channels_cfg_valid[i] == 1u ) {
				auint8_t rb = Com_Channel_Read(i);
				if ( !rb.len ){
					continue;
				}
				diskRequest.reqArg.write_request.fptr = &logFiles[i];
				diskRequest.reqArg.write_request.dptr = rb.ptr;
				diskRequest.reqArg.write_request.btw = rb.len;
				diskRequest.reqType = e_diskt_write_file;
				ProcStatus_t ret  = DISKT_PostRequest(diskRequest);
				_UNUSED_(ret);
			}
		}
	}

	/* Flush Pipe  */

	if ( fsmt.cnt%((COM_CHANNEL_FLUSH_PERIOD_MS)/FSM_CYCLE_DURATION_MS) == 0u ){

		for ( uint8_t i = 0 ; i < COM_CHANNEL_LENGTH ; i++ ){
			if ( com_channels_cfg_valid[i] == 1u ) {
				diskRequest.reqArg.flush_request.fptr = &logFiles[i];
				diskRequest.reqType = e_diskt_flush_file;
				ProcStatus_t ret  = DISKT_PostRequest(diskRequest);
				_UNUSED_(ret);
			}
		}

	}


	fsmt.ns = e_fsm_state_monitor;
	fsmt.err = e_err_ok;

}

static void fsm_default_h ( void ){


	Common_Printf("FSM_STATE_UNKNOWN\r\n");
}


static void fsm_illuminate_state( void ){

	if ( fsmt.cnt%(2u*(fsm_states[fsmt.cs].togglePeriod/FSM_CYCLE_DURATION_MS)) == 0U ){
		RGB_Write(fsm_states[fsmt.cs].colorCycle1);
	}
	else if ( fsmt.cnt%(1u*(fsm_states[fsmt.cs].togglePeriod/FSM_CYCLE_DURATION_MS)) == 0U ){
		RGB_Write(fsm_states[fsmt.cs].colorCycle2);
	}

}

static void fsm_err_h ( void ){
	Common_Printf("FSM_STATE_ERR[%d]\r\n",fsmt.err);

	static uint32_t err_cnt = 0u;

	if ( fsmt.ini == 1u ){
		err_cnt = 0u;
	}
	if ( err_cnt < 50u ){
		fsmt.ns = e_fsm_state_error;
		err_cnt++;
		return;

	}
	switch( fsmt.err ){

	case e_err_mount_failed:
		bool cardStatus = SDCD_isCardInserted();
		if ( cardStatus == true ){
			fsmt.ns = e_fsm_state_enumerate_hardware;
			fsmt.err = e_err_ok;
		}
		break;
	default:
		fsmt.ns = e_fsm_state_error;
		break;
	}
}

extern void fsm_init( void ){
	jsmn_init(&json_parser); /* Initialize jsmn jason parser */

	fsmt.cs = e_fsm_state_init;
	fsmt.ns = e_fsm_state_init;
	fsmt.ps = e_fsm_state_init;
	fsmt.ini = false;
	fsmt.cnt = 0u;
	fsmt.err = e_err_ok;
}

extern void fsm ( void ){

	fsmt.ps = fsmt.cs;
	fsmt.cs = fsmt.ns ;

	fsmt.ini =  fsmt.cs != fsmt.ps;

	fsm_illuminate_state();

	switch(fsmt.cs){

	case e_fsm_state_init:
		fsm_init_h();
	break;
	case e_fsm_state_enumerate_hardware:
		fsm_enumerate_hardware_h();
	break;
	case e_fsm_state_load_configuration:
		fsm_load_configuration_h();
	break;
	case e_fsm_state_enum_configuration:
		fsm_enum_configuration_h();
	break;
	case e_fsm_state_open_log_files:
		fsm_open_logs_h();
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

	fsmt.cnt++;

}
