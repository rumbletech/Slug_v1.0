#include "common.h"
#include "fatfs.h"
#include <stdbool.h>
#include "SDCD.h"
#include "diskio.h"
#include "lw_gpio.h"
#include "lw_spi.h"
#include "lw_rcc.h"
#include "lw_sys.h"
#include "bsp.h"

#define _SDCD_ERRH_ 0x02

 struct SDCD_Data_s {
	 lw_spi spi; /* SPI Device */
	 lw_gpio cs; /* SPI CS GPIO Device */
	 lw_gpio mosi; /* SPI MOSI GPIO Device */
	 lw_gpio miso; /* SPI MISO GPIO Device */
	 lw_gpio sclk; /* SPI SCLK GPIO Device */
	 lw_gpio cdet; /* Card Detect pin */
	 SD_Card_Type mType; /* Driver Detected Memory Type */
	 DSTATUS mStat; /* Driver Status */
	 bool pwrf; /* Driver Power Flag */

 };

static struct SDCD_Data_s sdcd;


static const uint16_t ccitt_hash[] = {
    0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
    0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
    0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
    0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
    0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
    0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
    0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
    0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
    0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
    0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
    0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
    0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
    0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
    0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
    0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
    0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
    0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
    0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
    0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
    0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
    0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
    0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
    0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
    0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
    0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
    0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
    0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
    0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
    0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
    0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
    0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
    0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0,
};

uint16_t crc16_ccitt(const uint8_t* buffer, size_t size){
    uint16_t crc = 0;
    while (size-- > 0)
    {
    	crc = (crc << 8) ^ ccitt_hash[((crc >> 8) ^ *(buffer++)) & 0x00FF];
    }
    return crc;
}

static inline uint8_t crc7(SD_Card_CMD cmd_idx){
	if(cmd_idx == SDC_CMD_GO_IDLE_STATE ){
		return 0x95;
	}
	else if (cmd_idx == SDC_CMD_SEND_IF_COND){
		return 0x87;
	}
	return 0x01;
}

static _sdcd_err SDCD_Receive(uint8_t* ptr , uint16_t len ){
	lw_SPI_TransmitReceieve(&sdcd.spi, NULL , ptr , len);
	return SDCD_SUCCESS;
}

static _sdcd_err SDCD_Transmit(uint8_t* ptr , uint16_t len ){
	lw_SPI_TransmitReceieve(&sdcd.spi, ptr , NULL , len);
	return SDCD_SUCCESS;
}

static void SDCD_SelectChip( void ){
	lw_GPIO_Write(&sdcd.cs,0u);
}

static void SDCD_DeselectChip( void ){
	lw_GPIO_Write(&sdcd.cs,1u);
}

static void SDCD_PreInit( void ){
	lw_RCC_Enable_SPI1();
	lw_RCC_Enable_GPIOA();
}

extern bool SDCD_isCardInserted ( void ){
	return !lw_GPIO_Read(&sdcd.cdet);
}

void SDCD_BSP_Init( void ){
	sdcd.cs.hwctx = BSP_SDC_SPI_CS_PORT;
	sdcd.cs.data.pin = BSP_SDC_SPI_CS_PIN;
	sdcd.cs.data.cfg = LW_GPIO_CFG_OUTPUT_PP;
	sdcd.cs.data.mode = LW_GPIO_MODE_50MHZ;
	lw_GPIO_Init(&sdcd.cs);

	sdcd.mosi.hwctx = BSP_SDC_SPI_MOSI_PORT;
	sdcd.mosi.data.pin = BSP_SDC_SPI_MOSI_PIN;
	sdcd.mosi.data.cfg = LW_GPIO_CFG_ALT_PP;
	sdcd.mosi.data.mode = LW_GPIO_MODE_10MHZ;
	lw_GPIO_Init(&sdcd.mosi);

	sdcd.sclk.hwctx = BSP_SDC_SPI_SCLK_PORT;
	sdcd.sclk.data.pin = BSP_SDC_SPI_SCLK_PIN;
	sdcd.sclk.data.cfg = LW_GPIO_CFG_ALT_PP;
	sdcd.sclk.data.mode = LW_GPIO_MODE_10MHZ;
	lw_GPIO_Init(&sdcd.sclk);

	sdcd.miso.hwctx = BSP_SDC_SPI_MISO_PORT;
	sdcd.miso.data.pin = BSP_SDC_SPI_MISO_PIN;
	sdcd.miso.data.cfg = LW_GPIO_CFG_INPUT_FLOAT;
	sdcd.miso.data.mode = LW_GPIO_MODE_INPUT;
	lw_GPIO_Init(&sdcd.miso);

	sdcd.cdet.hwctx = BSP_SDC_CDET_PORT;
	sdcd.cdet.data.pin = BSP_SDC_CDET_PIN;
	sdcd.cdet.data.cfg = LW_GPIO_CFG_INPUT_PULL;
	sdcd.cdet.data.mode = LW_GPIO_MODE_INPUT;
	sdcd.cdet.data.pud = LW_GPIO_PUD_PULL_UP;
	lw_GPIO_Init(&sdcd.cdet);

	sdcd.spi.hwctx = BSP_SDC_SPI;
	lw_SPI_Init(&sdcd.spi);
}


void SDCD_Init ( void ){

	SDCD_PreInit();

	SDCD_BSP_Init();

	sdcd.mStat = STA_NOINIT;
	sdcd.pwrf = false;
	sdcd.mType = SDC_UNKNOWN;

	SDCD_DeselectChip();
}

/* power off */
static inline void SD_PowerOff(void){
	sdcd.pwrf = false;
}

static _sdcd_err SD_ReadyWait(void){
	uint8_t res = 0u;
	_sdcd_err err = SDCD_SUCCESS;
	uint16_t timeOut = 1000;
	uint32_t elapsedTicks;
	uint32_t currentTicks = lw_Sys_Get_Ticks();

	do {
		err = SDCD_Receive(&res,1u);
		elapsedTicks = lw_Sys_Get_Ticks()-currentTicks;
	} while ( (res != 0xFF) && (elapsedTicks < timeOut) && (err == SDCD_SUCCESS));

	if ( elapsedTicks >= timeOut ){
		err |= SDCD_READY_TIMEOUT;
	}

	return err;
}

static _sdcd_err SD_SendCmd(SD_Card_CMD cmd , uint32_t arg , uint8_t* ret){
	_sdcd_err err = SDCD_SUCCESS;
	uint8_t n = 20;
	uint8_t res = 0;
	uint32_t cnt = 0x1fff;

	if ( cmd !=  SDC_CMD_GO_IDLE_STATE ){
		err = SD_ReadyWait();

		if ( err != SDCD_SUCCESS ){
			goto SD_SendCmd_exit_;
		}
	}

	uint8_t command[6];
	command[0] = 0x40 | (uint8_t)cmd;
	command[1] = (uint8_t)(arg >> 24) ;
	command[2] = (uint8_t)(arg >> 16) ;
	command[3] = (uint8_t)(arg >> 8) ;
	command[4] = (uint8_t)(arg) ;
	command[5] = crc7(cmd);

	err = SDCD_Transmit(&command[0],6);

	if ( err != SDCD_SUCCESS ){
		Common_Printf("failed here 3\r\n");
		goto SD_SendCmd_exit_;
	}

	if( cmd == SDC_CMD_STOP_TRANSMISSION ) {
		err = SDCD_Receive(&res , 1u);
		if ( err != SDCD_SUCCESS ){
			Common_Printf("failed here 4\r\n");
			goto SD_SendCmd_exit_;
		}
	}


	if(cmd == SDC_CMD_GO_IDLE_STATE) {
		while ((res != 0x01) && cnt)
		{
			SDCD_Receive(&res,1);
			cnt--;
		}
	}
	else {
		do {
			err = SDCD_Receive(&res , 1);
		} while ((res & 0x80) && n--);


		if ( n  == 0u ){
			Common_Printf("failed here 5\r\n");
			err|= SDCD_CMD_TIMEOUT;
		}
	}

	*ret = res;

SD_SendCmd_exit_:

	if ( err != SDCD_SUCCESS ){
		Common_Printf("failed here 6\r\n");
		err |= SDCD_CMD_FAIL;
	}

	return err;
}


static inline uint8_t SD_CheckPower(void){
	return sdcd.pwrf;
}



static bool SD_PowerOn(){

	lw_Sys_Delay(5000u);
	uint8_t dummyBytes[10] = { [ 0 ... 9 ] = 0xFF };
	uint8_t res ;
	sdcd.pwrf = false;
	_sdcd_err err = SDCD_SUCCESS;

	if ( sdcd.pwrf == true ) {
		goto SD_PowerOn_exit_;
	}

	SDCD_DeselectChip();

	err = SDCD_Transmit(&dummyBytes[0],sizeof(dummyBytes));
	if ( err != SDCD_SUCCESS ){
		goto SD_PowerOn_exit_;
	}

	SDCD_SelectChip();
	err = SD_SendCmd(SDC_CMD_GO_IDLE_STATE,0,&res);
	SDCD_DeselectChip();

	if ( err != SDCD_SUCCESS || res != 0x01){
		goto SD_PowerOn_exit_;
	}

	if( res == 0x01){
		sdcd.pwrf = true;
	}

SD_PowerOn_exit_:
	return sdcd.pwrf;
}

DSTATUS SD_disk_init(BYTE drv)
{
	_sdcd_err err = SDCD_SUCCESS;
	uint8_t res = 0u;
	uint8_t OCR[4u];


	if(drv){
		return STA_NOINIT;
	}

	if(sdcd.mStat & STA_NODISK){
		return sdcd.mStat;
	}
	
	if(SD_PowerOn() == false){
		sdcd.mType = SDC_UNKNOWN ;
		return STA_NOINIT;
	}

	SDCD_SelectChip();

	err = SD_SendCmd(SDC_CMD_SEND_IF_COND,0x000001AA,&res);

	if ( err != SDCD_SUCCESS ){
		goto SD_disk_init_exit;
	}

	err = SDCD_Receive(&OCR[0], sizeof(OCR));

	if ( err != SDCD_SUCCESS ){
		goto SD_disk_init_exit;
	}

	if( res == 1u &&
		OCR[2] == 0x01 &&
		OCR[3] == 0xAA ){

		do {
			err = SD_SendCmd(SDC_CMD_APP_CMD, 0 , &res);

			if ( err != SDCD_SUCCESS ){
				goto SD_disk_init_exit;
			}

			err = SD_SendCmd(SDC_CMD_SEND_OP_COND,0x40000000 , &res);

			if ( err != SDCD_SUCCESS ){
				goto SD_disk_init_exit;
			}
		}
		while( res != 0u );

		err = SD_SendCmd(SDC_CMD_READ_OCR,0,&res);

		if ( err != SDCD_SUCCESS ){
			goto SD_disk_init_exit;
		}

		err = SDCD_Receive(OCR, sizeof(OCR));

		if ( err != SDCD_SUCCESS ){
			goto SD_disk_init_exit;
		}

		if(OCR[0u] && 0x40){  //******************
			sdcd.mType = SDC_HIGH_CAPACITY;
		}
		else{
			sdcd.mType = SDC_STANDARD;
		}
	}
	else {
		/* SDC V1 or MMC */
		err = SD_SendCmd(SDC_CMD_APP_CMD, 0,&res);

		if ( err != SDCD_SUCCESS ){
			goto SD_disk_init_exit;
		}

		if ( res > 1 ){
			//sdcd.mType = MMC_V3;
			goto SD_disk_init_exit;
		}

		err = SD_SendCmd(SDC_CMD_SEND_OP_COND, 0,&res);

		if ( err != SDCD_SUCCESS ){
			goto SD_disk_init_exit;
		}

		if ( res <= 1u ){
			sdcd.mType = SDC_MMC;
		}
		else {
			sdcd.mType = SDC_MMC;
		}

	}

	if( sdcd.mType  == SDC_STANDARD ||
	    sdcd.mType  == SDC_MMC){

		if ( sdcd.mType == SDC_MMC ){
			err = SD_SendCmd(SDC_CMD_SEND_OP_COND, 0, &res);
			if ( err != SDCD_SUCCESS ){
				goto SD_disk_init_exit;
			}
		}
		err = SD_SendCmd(SDC_CMD_SET_BLOCKLEN, SDCD_BLOCK_LEN , &res);
		if ( err != SDCD_SUCCESS ){
			goto SD_disk_init_exit;
		}
	}

SD_disk_init_exit:
	SDCD_DeselectChip();
	/* Clear STA_NOINIT */
	if (sdcd.mType != SDC_UNKNOWN ){
		sdcd.mStat &= ~STA_NOINIT;
	}
	else{
		sdcd.mStat |=  STA_NOINIT;
		SD_PowerOff();
	}
	return sdcd.mStat;
}

static bool SD_TxDataBlock(const BYTE *buff,uint8_t token)
{

	uint8_t dummybyte = 0xff;
	uint8_t response;
	_sdcd_err err = SDCD_SUCCESS;
	(void)err;

	err = SD_ReadyWait();

	if ( err != SDCD_SUCCESS ){
		return false;
	}
	
	//Send token
	SDCD_Transmit(&token,1);
	//Check the token 
	if(token != 0xFD){

		//Send Data block
		SDCD_Transmit((uint8_t *)buff,512);
		//Send Two dummy bytes inplace of CRC bytes
		SDCD_Transmit(&dummybyte,1);
		SDCD_Transmit(&dummybyte,1);
		//Check the response
		for(int i = 0 ; i<= 64 ; i++ ){
			err = SDCD_Receive(&response,1);
			if((response & 0x1F) == 0x05 ) //If lower nibble of the response equals 5 then the data was received successfully
				break; 
		}
	}

	if((response & 0x1F) == 0x05 ){
		return true;
	}

	Common_Printf("failed here 2\r\n");

	return false;
}

static bool SD_RxDataBlock(BYTE *buff, uint32_t len){

	uint8_t crc[2u];
	/* timeout 200ms */
	uint16_t timeOut = 200;
	uint8_t res = 0u;
	uint32_t elapsedTicks;
	uint32_t currentTicks = lw_Sys_Get_Ticks();
	_sdcd_err err = SDCD_SUCCESS;
	/* loop until receive a response or timeout */
	do {
		err = SDCD_Receive(&res, 1u);
		elapsedTicks = lw_Sys_Get_Ticks()-currentTicks;
	} while( ( res != 0xFE) &&
			(elapsedTicks < timeOut));

	if(res != 0xFE){
		return false;
	}

	if ( elapsedTicks >= timeOut ){
		Common_Printf("Timeout During Read \r\n");
		return false;
	}

	err = SDCD_Receive(buff,len);

	if ( err != SDCD_SUCCESS ){
		Common_Printf("Error During Read byte \r\n");
		return false;
	}

	err = SDCD_Receive(crc, sizeof(crc));

	uint16_t r_crc = (((uint16_t)crc[0u]) << 8u) | crc[1u];
	uint16_t cal_crc = crc16_ccitt(buff, len);

	if ( err != SDCD_SUCCESS ){
		Common_Printf("Error During Read CRC\r\n");
		return false;
	}

	if ( cal_crc != r_crc ){
		return false;
	}

	return true;
}

DRESULT SD_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count){
	uint8_t res = 0;
	_sdcd_err err = SDCD_SUCCESS;
	(void)err;
	/* pdrv should be 0 */
	if (pdrv || !count) {  return RES_PARERR; }

	/* no disk */
	if (sdcd.mStat & STA_NOINIT){ return RES_NOTRDY; }

	/* write protection */
	if (sdcd.mStat & STA_PROTECT){ return RES_WRPRT; }

	/* convert to byte address */
	if (sdcd.mType == SDC_STANDARD){ sector *= 512; }

	SDCD_SelectChip();

	if (count == 1){
		Common_Printf("Singular \r\n");

		err = SD_SendCmd(SDC_CMD_WRITE_BLOCK, sector,&res);
		/* WRITE_BLOCK */
		if (res == 0) { //Send CMD 24 WRITE_SINGLE_BLOCK
			if(SD_TxDataBlock(buff, 0xFE)){ //Send datablock stored in the buffer , with CMD 24 token 0xFE
				count = 0;
			}
		}
	}
	else
	{
		/* WRITE_MULTIPLE_BLOCK */
		if (sdcd.mType == SDC_STANDARD)
		{
			SD_SendCmd(SDC_CMD_SET_BLOCK_COUNT, 0,&res);   // For MMC, the number of blocks to write must be pre-defined by CMD23(SET_BLOCK_COUNT) prior to CMD25(WRITE_MULTIPLE_BLOCKS)
									//	and the write transaction is terminated at last data block.
			SD_SendCmd(SDC_CMD_WRITE_MULTIPLE_BLOCK, count,&res); /* ACMD23 */
		}

		if (SD_SendCmd(SDC_CMD_WRITE_MULTIPLE_BLOCK, sector,&res) == 0) //send CMD 25 with starting sector address
		{
			do {
				if(!SD_TxDataBlock(buff, 0xFC)) break; // if SDCARD timesout and doesnt respond or data was transmitted unsuccessfully then break

				buff += 512; //increment the buffer pointer to the starting address of the next block to be written
			} while (--count); //decrement the total number of blocks to be written by 1

			/* STOP_TRAN token */
			SD_TxDataBlock(0, 0xFD);//This is specific to SD cards only ,
									//For SD CARDS a multiple block write must be terminated by a STOP TRANSACTION token
			if(!SD_SendCmd(SDC_CMD_SEND_STATUS, 0,&res) == 0) //Check status of SDCARD if response = 0x00 then it successfully transmitted the STOP TRAN TOKEN

			{
				count = 1; //if the response is other than 0x00 then an error occured
			}
		}
	}

	/* Idle */
	SDCD_DeselectChip();
	SDCD_Receive(&res,1);

	return count ? RES_ERROR : RES_OK;
}

DRESULT SD_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count){

	_sdcd_err err = SDCD_SUCCESS;
	uint8_t res = 0u;
	/* pdrv should be 0 */
	if (pdrv || !count){
		return RES_PARERR;
	}

	/* no disk */
	if (sdcd.mStat & STA_NOINIT){
		return RES_NOTRDY;
	}

	/* convert to byte address */
	if (sdcd.mType == SDC_STANDARD){
		sector *= 512;
	}

	SDCD_SelectChip();

	if (count == 1u)
	{
		err = SD_SendCmd(SDC_CMD_READ_SINGLE_BLOCK, sector,&res);
		if ( err != SDCD_SUCCESS || res != 0u ){
			goto SD_disk_read_exit;
		}

		if ( SD_RxDataBlock(buff, 512) ) {
			count = 0;
		}
		else{
			err = SDCD_READ_FAIL;
			goto SD_disk_read_exit;
		}
	}
	else
	{
		err = SD_SendCmd(SDC_CMD_READ_MULTIPLE_BLOCK, sector,&res);

		if ( err != SDCD_SUCCESS || res != 0u ){
			goto SD_disk_read_exit;
		}

		do {
			if (!SD_RxDataBlock(buff, 512)){
				break;
			}
			buff += 512;
		} while (--count);

		if ( count != 0u ){
			err = SDCD_READ_FAIL;
			goto SD_disk_read_exit;
		}

		/* STOP_TRANSMISSION */
		err = SD_SendCmd(SDC_CMD_STOP_TRANSMISSION, 0 , &res);

		if ( err != SDCD_SUCCESS ){
			goto SD_disk_read_exit;
		}

	}

SD_disk_read_exit:
	/* Idle */
	SDCD_DeselectChip();

	return err != SDCD_SUCCESS ? RES_ERROR : RES_OK;
}


DRESULT SD_disk_ioctl(BYTE drv, BYTE ctrl, void *buff){
	DRESULT res;
	uint8_t n, csd[16], *ptr = buff;
	WORD csize;
	uint8_t response ;
	_sdcd_err sdc_err;

	if (drv){
		return RES_PARERR;
	}

	res = RES_ERROR;

	if (ctrl == CTRL_POWER)
	{
		switch (*ptr)
		{
		case 0:
			SD_PowerOff();		/* Power Off */
			res = RES_OK;
			break;
		case 1:
			SD_PowerOn();		/* Power On */
			res = RES_OK;
			break;
		case 2:
			*(ptr + 1) = SD_CheckPower();
			res = RES_OK;		/* Power Check */
			break;
		default:
			res = RES_PARERR;
		}
	}
	else
	{
		if (sdcd.mStat & STA_NOINIT){
			return RES_NOTRDY;
		}

		SDCD_SelectChip();

		switch (ctrl)
		{
		case GET_SECTOR_COUNT:
		{
			/* SEND_CSD */
			SD_SendCmd(SDC_CMD_SEND_CSD, 0,&response);

			if (( response == 0) && SD_RxDataBlock((BYTE*) csd, 16))
			{
				if ((csd[0] >> 6) == 1)
				{
					/* SDC V2 */
					csize = csd[9] + ((WORD) csd[8] << 8) + 1; //Get the first 16 most signifcant bytes of the CSD register
					*(DWORD*) buff = (DWORD) csize << 10;		// Bytes stored in csd[8] csd[9] form the MSB and LSB respectively of the of the device's size C_SIZE
				}												// C_SIZE gives the device's size in terms of Blocks where 1 Unit of C_SIZE equals 1024 Block
																// Each Block is 512 bytes
				else											// multiply C_SIZE by 1024 to get the total number of blocks(sectors)
				{
					/* MMC or SDC V1 */
					n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
					csize = (csd[8] >> 6) + ((WORD) csd[7] << 2) + ((WORD) (csd[6] & 3) << 10) + 1;
					*(DWORD*) buff = (DWORD) csize << (n - 9);
				}
				res = RES_OK;
			}
		}
		break;
		case GET_SECTOR_SIZE:
		{
			*(WORD*) buff = 512;
			res = RES_OK;
		}
		break;
		case CTRL_SYNC:
		{
			sdc_err =  SD_ReadyWait();
			if ( sdc_err == SDCD_SUCCESS ){
				res = RES_OK;
			}
		}
		break;
		case MMC_GET_CSD: //Get the first 16 most signifcant bytes of the CSD register
			/* SEND_CSD */
		{
			SD_SendCmd(SDC_CMD_SEND_CSD, 0,&response);
			if (response == 0 && SD_RxDataBlock((BYTE*)ptr, 16)){
				res = RES_OK;
			}
		}
		break;
		case MMC_GET_CID: //Get the first 16 most signifcant bytes of the CID register
			/* SEND_CID */
		{
			SD_SendCmd(SDC_CMD_SEND_CSD, 0,&response);
			if (response == 0 && SD_RxDataBlock((BYTE*)ptr, 16)){
				res = RES_OK;
			}
		}
		break;
		case MMC_GET_OCR:
		{
			/* READ_OCR */
			SD_SendCmd(SDC_CMD_READ_OCR, 0,&response);
			if (response == 0)
			{
				for (n = 0; n < 4; n++)
				{
					SDCD_Receive((ptr + n),1);
				}
				res = RES_OK;
			}
		}
		break;
		default:
			res = RES_PARERR;
		}

		SDCD_DeselectChip();
		SDCD_Receive(ptr,1);
	}

	return res;
}

DSTATUS SD_disk_status (BYTE pdrv){
	return (DSTATUS) sdcd.mStat;
}

