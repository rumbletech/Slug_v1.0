/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include <com_channel.h>
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "common.h"
#include "SDCD.h"
#include "rgb.h"
#include "lw_sys.h"
#include "com_channel.h"
#include "jsmn.h"
#include "fatfs.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define JSON_TOK_MAX_SIZE 128U
#define FS_FORCE_MOUNT 1u
#define FS_LABEL_SIZE 128u
#define FS_CONFIG_MAX_BUFF_SIZE 512u

static lw_uart_data_t com_channels_cfg[COM_CHANNEL_LENGTH];
static bool com_channels_cfg_valid[COM_CHANNEL_LENGTH];
static jsmn_parser json_parser;
static jsmntok_t json_tokens[JSON_TOK_MAX_SIZE];
static const uint32_t supported_baud_rates[] = {1200,2400,4800,9600,14400,19200,28800,38400,57600,115200,128000,230400,250000,460800,500000};
static TCHAR label[FS_LABEL_SIZE];
static DWORD volumeSerialNumber;
static FATFS ffs;

static FIL configJsonFile;
static uint8_t configJsonBuffer[512u];
static uint32_t configJsonFileLength;



//static bool assertBaud( uint32_t baudRate ){
//	bool ret = false;
//	for ( uint32_t i = 0 ; i < sizeof(supported_baud_rates)/sizeof(uint32_t) ;i++ ){
//		if ( baudRate == supported_baud_rates[i]){
//			ret = true;
//			break;
//		}
//	}
//	return ret;
//}

//static bool assertConfiguration( lw_uart_data_t cfg ){
//
//	if ( !assertBaud(cfg.baudRate)  ){
//		return false;
//	}
//
//	if ( cfg.parity >= e_uch_parity_MAX_ ){
//		return false;
//	}
//
//	if ( cfg.nStopBits >= e_uch_nStopbits_MAX_ ){
//		return false;
//	}
//
//	if ( cfg.nDataBits >= e_uch_nDatabits_MAX_ ){
//		return false;
//	}
//
//	return true;
//}

extern volatile uint32_t uart_count;
#define UCH_CONFIG_MAX_TOKEN_SIZE 64u


/* Init json parser */
//	jsmn_init(&uch.json.parser);

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

	uint32_t jsnFileLen = configJsonFileLength;
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
				uint32_t key_sz = json_tokens[key].end-json_tokens[key].start;
				uint8_t value = i+j+2u;
				uint32_t value_sz = json_tokens[value].end - json_tokens[value].start;
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

static void TEST_SDCARD( void ){
	static uint32_t index = 0u;
	static uint32_t failCount = 0u;

	BYTE sec[512u];
	failCount += (uint32_t)SD_disk_read(0, &sec[0u], index+8192 , 1);
	Common_Printf("Sector %d = \r\n" , index);
	Common_Printf("Total Fail %d = \r\n" , failCount);

	index++;
}

static void TEST_RGB ( void ){

      static uint32_t color = 0u;
	  RGB_Write(color++);
}

/* Executed as First Step During Startup ( Before Main ) */
extern void SystemInit ( void ){
	/* Init NVIC , System Clock and SysTick */
	lw_Sys_Init();
}


static void fsys_init ( void ){

	FRESULT res = f_mount(&ffs, "0:/", FS_FORCE_MOUNT);

	if ( res != FR_OK ){
		Common_Printf("f_mount failed %d\r\n", res);
		return;
	}

	res = f_getlabel("",&label[0u], &volumeSerialNumber);

	if ( res != FR_OK ){
		Common_Printf("f_getlabel failed %d\r\n", res);
		return;
	}

	Common_Printf("Loading Successful\r\n");

	Common_Printf("SDC Detected : \r\n");
	Common_Printf("fs_id : %d\r\n" , ffs.id);
	Common_Printf("fs_type : %d\r\n" , ffs.fs_type);
	Common_Printf("fs_drv : %d\r\n" , ffs.drv);
	Common_Printf("fs_fsize : %d\r\n" , ffs.fsize);
	Common_Printf("fs_csize : %d\r\n" , ffs.csize);
	Common_Printf("fs_free_clust : %d\r\n" , ffs.free_clust);
	Common_Printf("fs_label : %s\r\n" , &label[0u]);
	Common_Printf("fs_serialNumber : %d\r\n" , volumeSerialNumber);

}

static void fsys_load_config( void ){

	UINT rb = 0U;
	FRESULT res;

#if _USE_LFN != 0
    res = f_open(&configJsonFile,"0:/slug/config.json",FA_READ);
#else
    res = f_open(&configJsonFile,"0:/slug/CONFIG~1.JSO",FA_READ);
#endif
	if ( res != FR_OK ){
		Common_Printf("f_open failed %d \r\n" , res);
		return;
	}

	res = f_read(&configJsonFile,&configJsonBuffer[0u],sizeof(configJsonFile),&rb);

	if ( res != FR_OK ){
		Common_Printf("f_read failed %d\r\n" , res);
		return;
	}

	configJsonFileLength = rb;

	Common_Printf("FileSize : %d \r\n  File : %s \r\n", rb , configJsonBuffer);
	Common_Printf("Strlen : %d \r\n" , strlen(&configJsonBuffer[0u]));

	res = f_close(&configJsonFile);
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  (void)(TEST_SDCARD);
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  SystemInit();
  //HAL_MspInit();
  /* USER CODE BEGIN Init */

  //SystemClock_Config();
  /* USER CODE END Init */

  /* Configure the system clock */
 // SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
//  MX_GPIO_Init();
//  MX_SPI1_Init();
//  MX_USART1_UART_Init();
//  MX_USART2_UART_Init();
//  huart3.hwctx = _DEBUG_PRINTF_PORT_;
//  huart3.data.baudRate = _DEBUG_PRINTF_BAUD_;
//  huart3.data.dataBits = _DEBUG_PRINTF_WLEN_;
//  huart3.data.stopBits = _DEBUG_PRINTF_STOPB_;
//  huart3.data.parity = LW_UART_PARITY_NONE;
//  lw_UART_Init(&huart3);
  MX_FATFS_Init();
//  MX_RTC_Init();
//  MX_USB_PCD_Init();
  /* USER CODE BEGIN 2 */

  RGB_Init();
  RGB_Write(COLOR_CAYAN);
  SDCD_Init();
  Com_Channel_Init();
  jsmn_init(&json_parser);
  fsys_init();


  /* Load Config File into Memory */

//#if _USE_LFN != 0
//    res = f_open(&fsys.json.file,"0:/slug/config.json",FA_READ);
//#else
//    res = f_open(&fsys.json.file,"0:/slug/CONFIG~1.JSO",FA_READ);
//#endif
  fsys_load_config();
  /* Enumerate Configuration from SD Card */
  com_channels_cfg_valid[COM_CHANNEL_1] = false;
  com_channels_cfg_valid[COM_CHANNEL_2] = false;
  com_channels_cfg_valid[COM_CHANNEL_3] = false;

  Enumerate_COM_Channels();

  if ( com_channels_cfg_valid[COM_CHANNEL_1] ){
	  Com_Channel_Configure(COM_CHANNEL_1, com_channels_cfg[COM_CHANNEL_1]);
	  Com_Channel_Enable(COM_CHANNEL_1);
  }
  else{
	  Com_Channel_StopLogging(COM_CHANNEL_1);
	  Com_Channel_Disable(COM_CHANNEL_1);
  }

  if ( com_channels_cfg_valid[COM_CHANNEL_2] ){
	  Com_Channel_Configure(COM_CHANNEL_2, com_channels_cfg[COM_CHANNEL_2]);
	  Com_Channel_Enable(COM_CHANNEL_2);
  }
  else{
	  Com_Channel_StopLogging(COM_CHANNEL_2);
	  Com_Channel_Disable(COM_CHANNEL_2);
  }

  if ( com_channels_cfg_valid[COM_CHANNEL_3] ){
	  Com_Channel_Configure(COM_CHANNEL_3, com_channels_cfg[COM_CHANNEL_3]);
	  Com_Channel_Enable(COM_CHANNEL_3);

  }
  else{
	  Com_Channel_StopLogging(COM_CHANNEL_3);
	  Com_Channel_Disable(COM_CHANNEL_3);
  }

  /* Enumerate Channels from SDC */

//  uch_ApplyConfiguration(CHANNEL_1);
//  uch_ApplyConfiguration(CHANNEL_2);
//  uch_ApplyConfiguration(CHANNEL_3);

  Common_Printf("Enumeration Successful Debug Port Open\r\n");

  //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
	//  TEST_SDCARD();
	  TEST_RGB();
	  lw_Sys_Delay(1000u);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
