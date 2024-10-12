#include "common.h"
#include "fatfs.h"
#include "GPIO.h"
#include <stdbool.h>
#include "SDCD.h"

#define SPI_TIMEOUT 100

#define _SDCD_ERRH_ 0x02

#define SDCD_SUCCESS       0x0000
#define SDCD_HAL_FAIL      0x0001
#define SDCD_HAL_TIMEOUT   0x0002
#define SDCD_CMD_FAIL      0x0004
#define SDCD_CMD_TIMEOUT   0x0008
#define SDCD_READY_TIMEOUT 0x0010

#define SDCD_BLOCK_LEN 512u

typedef uint16_t _sdcd_err;

enum memd_cmd {
	          GO_IDLE_STATE,
			  SEND_OP_COND,
			  APP_SEND_OP_COND = 41,
			  SEND_IF_COND = 8,
			  SEND_CSD,
			  SEND_CID,
			  STOP_TRANSMISSION = 12,
			  SEND_STATUS = 13,
			  SET_BLOCKLEN = 16,
			  READ_SINGLE_BLOCK,
			  READ_MULTIPLE_BLOCK,
			  SET_BLOCK_COUNT = 23,
			  WRITE_BLOCK,
			  WRITE_MULTIPLE_BLOCK,
			  APP_CMD = 55,
			  READ_OCR = 58,
			  SET_WR_BLOCK_ERASE_COUNT
};

 enum memd_type {
	    UNKNOWN_CARD,
	 	MMC_V3,
		SD_V1,
		SD_V2HC,
		SD_V2SC
};



 struct SDCD_Data_s {
	 SPI_HandleTypeDef* spi_d;	/* SPID Device */
	 GPIO_HandleTypeDef* spi_cs; /* SPI CS GPIO Device */
	 enum memd_type mType; /* Driver Detected Memory Type */
	 DSTATUS mStat; /* Driver Status */
	 bool pwrf; /* Driver Power Flag */

 };

static struct SDCD_Data_s sdcd;


static inline void CS_Enable(){
	HAL_GPIO_WritePin(sdcd.spi_cs->port, 1u << sdcd.spi_cs->pinNum, GPIO_PIN_RESET);
}

static inline void CS_Disable(){
	HAL_GPIO_WritePin(sdcd.spi_cs->port, 1u << sdcd.spi_cs->pinNum, GPIO_PIN_SET);
}

void SDCD_Init (SPI_HandleTypeDef* spi_d ,  GPIO_HandleTypeDef* gpiod ){

	 if ( !spi_d ||
		  !gpiod ){
		 errh_code = _SDCD_ERRH_;
		 Error_Handler();
	 }

	 sdcd.spi_d = spi_d;
	 sdcd.spi_cs = gpiod;
	 sdcd.mStat = STA_NOINIT;
	 sdcd.pwrf = false;
	 sdcd.mType = UNKNOWN_CARD;

	 CS_Disable();
 }

static uint8_t get_CRC(uint8_t cmd_idx){
	if(cmd_idx == GO_IDLE_STATE ) return 0x95;
	else if (cmd_idx == SEND_IF_COND) return 0x87;
	else return 1;

}


static _sdcd_err SPI_Tx(uint8_t* data , uint16_t len){
	_sdcd_err err = SDCD_SUCCESS;

	while(!__HAL_SPI_GET_FLAG(sdcd.spi_d, SPI_FLAG_TXE));
	HAL_StatusTypeDef hal_ret = HAL_SPI_Transmit(sdcd.spi_d, data, len , SPI_TIMEOUT);

	if ( hal_ret != HAL_OK ){
		err = SDCD_HAL_FAIL;
		if ( hal_ret == HAL_TIMEOUT ){
			err = SDCD_HAL_TIMEOUT;
		}
	}
	return err;
}

static _sdcd_err SPI_Rx(uint8_t* data , uint16_t len){
	uint8_t dummy = 0xFF;
	_sdcd_err err = SDCD_SUCCESS;

	while(!__HAL_SPI_GET_FLAG(sdcd.spi_d, SPI_FLAG_TXE));
	HAL_StatusTypeDef hal_ret = HAL_SPI_TransmitReceive(sdcd.spi_d, &dummy, data , len , SPI_TIMEOUT);

	if ( hal_ret != HAL_OK ){
		err = SDCD_HAL_FAIL;
		if ( hal_ret == HAL_TIMEOUT ){
			err = SDCD_HAL_TIMEOUT;
		}
	}
	return err;
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
	uint32_t currentTicks = HAL_GetTick();

	do {
		err |= SPI_Rx(&res,1u);
		elapsedTicks = HAL_GetTick()-currentTicks;
	} while ( (res != 0xFF) && (elapsedTicks < timeOut) && (err == SDCD_SUCCESS));
	Common_Printf("res = %x\r\n" , res);

	if ( elapsedTicks >= timeOut ){
		Common_Printf("Wait Ready TIMEOUT\r\n");
		err |= SDCD_READY_TIMEOUT;
	}

	return err;
}

static _sdcd_err SD_SendCmd(enum memd_cmd cmd , uint32_t arg , uint8_t* ret){
	_sdcd_err err = SDCD_SUCCESS;
	uint8_t n = 10;
	uint8_t res = 0u;

	if ( cmd !=  GO_IDLE_STATE ){
		err = SD_ReadyWait();

		if ( err != SDCD_SUCCESS ){
			goto SD_SendCmd_exit_;
		}
	}

	uint8_t command[6u];
	command[0] = 0b01000000 | (uint8_t)cmd;
	command[1] = (uint8_t)(arg >> 24) ;
	command[2] = (uint8_t)(arg >> 16) ;
	command[3] = (uint8_t)(arg >> 8) ;
	command[4] = (uint8_t)(arg) ;
	command[5] = get_CRC(cmd);

	err = SPI_Tx(&command[0u],6u);

	if ( err != SDCD_SUCCESS ){
		goto SD_SendCmd_exit_;
	}

	if( cmd == STOP_TRANSMISSION ) {
		err = SPI_Rx(&res , 1u);
		if ( err != SDCD_SUCCESS ){
			goto SD_SendCmd_exit_;
		}
	}

	do {
		HAL_Delay(1);
		err = SPI_Rx(&res , 1u);
	} while ((res & 0x80) && (--n) && ( err == SDCD_SUCCESS));


	if ( n  == 0u ){
		err|= SDCD_CMD_TIMEOUT;
	}

	*ret = res;
	Common_Printf("cmd ret =%d\r\n",res);

SD_SendCmd_exit_:

	if ( err != SDCD_SUCCESS ){
		err |= SDCD_CMD_FAIL;
	}

	return err;
}

static inline uint8_t SD_CheckPower(void){
	return sdcd.pwrf;
}

static bool SD_PowerOn(){

	uint8_t dummyBytes[10u] = { 0xFF };
	uint8_t res = 0u;
	_sdcd_err err = SDCD_SUCCESS;

	if ( sdcd.pwrf == true ) {
		goto SD_PowerOn_exit_;
	}

	CS_Disable();
	HAL_Delay(500);
	CS_Enable();

	CS_Disable();

	err = SPI_Tx(&dummyBytes[0u],sizeof(dummyBytes));
	if ( err != SDCD_SUCCESS ){
		Common_Printf("SPI_Tx Fail \r\n");
		goto SD_PowerOn_exit_;
	}
	CS_Enable();
	err = SD_SendCmd(GO_IDLE_STATE,0,&res);
	CS_Disable();

	if ( err != SDCD_SUCCESS ){
		Common_Printf("GO_IDLE_STATE Fail %d\r\n" , err);
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
	uint32_t currentTicks , elapsedTicks , timeOut;
	_sdcd_err err = SDCD_SUCCESS;
	uint8_t res = 0u;
	uint8_t OCR[4u];

	Common_Printf("SD_disk_init\r\n");

	if(drv){
		Common_Printf("STA_NOINIT\r\n");
		return STA_NOINIT;
	}

	if(sdcd.mStat & STA_NODISK){
		Common_Printf("STA_NODISK\r\n");
		return sdcd.mStat;
	}
	
	if(SD_PowerOn() == false){
		Common_Printf("SD_PowerOn\r\n");
		sdcd.mType = UNKNOWN_CARD ;
		return STA_NOINIT;
	}

	CS_Enable();

	err = SD_SendCmd(SEND_IF_COND,0x000001AA,&res);

	if ( err != SDCD_SUCCESS ){
		goto SD_disk_init_exit;
	}
	Common_Printf("res = %d \r\n" , res);

	err = SPI_Rx(&OCR[0], sizeof(OCR));

	for ( int i = 0 ; i < 4 ; i++ ){
		Common_Printf("OCR[%d] = %d \r\n", i , OCR[i]);
	}

	if ( err != SDCD_SUCCESS ){
		goto SD_disk_init_exit;
	}

	if( res == 1u &&
		OCR[2] == 0x01 &&
		OCR[3] == 0xAA ){
		Common_Printf("V2HC,V2SC\r\n");

		err = SD_SendCmd(APP_CMD, 0 , &res);

		if ( err != SDCD_SUCCESS ){
			goto SD_disk_init_exit;
		}

		if ( res > 1u ){
			goto SD_disk_init_exit;
		}

		err = SD_SendCmd(APP_SEND_OP_COND,0x40000000 , &res);

		if ( err != SDCD_SUCCESS ){
			goto SD_disk_init_exit;
		}

		if ( res > 1u ){
			goto SD_disk_init_exit;
		}

		err = SD_SendCmd(READ_OCR,0,&res);

		if ( err != SDCD_SUCCESS ){
			goto SD_disk_init_exit;
		}


		if ( res != 1u ){
			goto SD_disk_init_exit;
		}

		err = SPI_Rx(OCR, sizeof(OCR));

		if ( err != SDCD_SUCCESS ){
			goto SD_disk_init_exit;
		}

		if(OCR[0u]==0x40){
			sdcd.mType = SD_V2HC;
		}
		else{
			sdcd.mType = SD_V2SC;
		}
	}
	else {
		/* SDC V1 or MMC */
		Common_Printf("SDC V1 or MMC\r\n");

		err = SD_SendCmd(APP_CMD, 0,&res);

		if ( err != SDCD_SUCCESS ){
			goto SD_disk_init_exit;
		}

		if ( res > 1 ){
			sdcd.mType = MMC_V3;
			goto SD_disk_init_exit;
		}

		err = SD_SendCmd(APP_SEND_OP_COND, 0,&res);

		if ( err != SDCD_SUCCESS ){
			goto SD_disk_init_exit;
		}

		if ( res <= 1u ){
			sdcd.mType = SD_V1;
		}
		else {
			sdcd.mType = MMC_V3;
		}

	}

	if(sdcd.mType  == SD_V1 ||
	   sdcd.mType  == SD_V2SC ||
	   sdcd.mType  == MMC_V3){

		if ( sdcd.mType == MMC_V3 ){
			err = SD_SendCmd(SEND_OP_COND, 0, &res);
			if ( err != SDCD_SUCCESS ){
				goto SD_disk_init_exit;
			}
		}
		err = SD_SendCmd(SET_BLOCKLEN, SDCD_BLOCK_LEN , &res);
		if ( err != SDCD_SUCCESS ){
			goto SD_disk_init_exit;
		}
	}
SD_disk_init_exit:
	CS_Disable();
	/* Clear STA_NOINIT */
	if (sdcd.mType != UNKNOWN_CARD ){
		Common_Printf("CLEAR STA_NOINIT\r\n");
		sdcd.mStat &= ~STA_NOINIT;
	}
	else{
		Common_Printf("SET STA_NOINIT\r\n");
		sdcd.mStat |=  STA_NOINIT;
		SD_PowerOff();
	}
	return sdcd.mStat;
}

static bool SD_TxDataBlock(const BYTE *buff,uint8_t token)
{

	uint8_t response;
	if(SD_ReadyWait() != 0xFF) {
		return false;
	}
	
	//Send token
	SPI_TxByte(token);
	//Check the token 
	if(token != 0xFD){

		//Send Data block
		SPI_TxBuffer((uint8_t *)buff,512);
		//Send Two dummy bytes inplace of CRC bytes
		SPI_TxByte(0xFF); 
		SPI_TxByte(0xFF);
		//Check the response
		for(int i = 0 ; i<= 64 ; i++ ){
			response = SPI_RxByte();
			if((response & 0x1F) == 0x05 ) //If lower nibble of the response equals 5 then the data was received successfully
				break; 
		}
		//Clear RX buffer
		while(__HAL_SPI_GET_FLAG(sdcd.spi_d,SPI_FLAG_RXNE)) {
			SPI_RxByte();
		}
	}

	if((response & 0x1F) == 0x05 ){
		return true;
	}

	return false;
}

static bool SD_RxDataBlock(BYTE *buff, uint32_t len){
	uint8_t crc[2u];
	/* timeout 200ms */
	uint16_t timeOut = 200;
	uint16_t res = 0u;
	uint32_t elapsedTicks;
	uint32_t currentTicks = HAL_GetTick();

	/* loop until receive a response or timeout */
	do {
		res = SPI_RxByte();
		if ( _SDCD_GET_STAT_(res) != SDCD_SUCCESS ){
			return false;
		}
		elapsedTicks = HAL_GetTick()-currentTicks;
	} while((_SDCD_GET_RESP_(res) != 0xFE) &&
			(elapsedTicks < timeOut));


	if ( elapsedTicks >= timeOut ){
		return false;
	}

	res = SPI_RxBuff_LI(buff, len);
	if ( _SDCD_GET_STAT_(res) != SDCD_SUCCESS ){
		return false;
	}

	res = SPI_RxBuff_LI(crc, sizeof(crc));
	if ( _SDCD_GET_STAT_(res) != SDCD_SUCCESS ){
		return false;
	}


	return true;
}

DRESULT SD_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count){
//	/* pdrv should be 0 */
//	if (pdrv || !count) return RES_PARERR;
//
//	/* no disk */
//	if (sdcd.mStat & STA_NOINIT) return RES_NOTRDY;
//
//	/* write protection */
//	if (sdcd.mStat & STA_PROTECT) return RES_WRPRT;
//
//	/* convert to byte address */
//	if (sdcd.mType == SD_V2SC) sector *= 512;
//
//	CS_Enable();
//
//	if (count == 1)
//	{
//		/* WRITE_BLOCK */
//		if ((SD_SendCmd(WRITE_BLOCK, sector) == 0)) //Send CMD 24 WRITE_SINGLE_BLOCK
//			if(SD_TxDataBlock(buff, 0xFE)) //Send datablock stored in the buffer , with CMD 24 token 0xFE
//			count = 0;
//	}
//	else
//	{
//		/* WRITE_MULTIPLE_BLOCK */
//		if (sdcd.mType == SD_V1)
//		{
//			SD_SendCmd(SET_BLOCK_COUNT, 0);   // For MMC, the number of blocks to write must be pre-defined by CMD23(SET_BLOCK_COUNT) prior to CMD25(WRITE_MULTIPLE_BLOCKS)
//									//	and the write transaction is terminated at last data block.
//			SD_SendCmd(WRITE_MULTIPLE_BLOCK, count); /* ACMD23 */
//		}
//
//		if (SD_SendCmd(WRITE_MULTIPLE_BLOCK, sector) == 0) //send CMD 25 with starting sector address
//		{
//			do {
//				if(!SD_TxDataBlock(buff, 0xFC)) break; // if SDCARD timesout and doesnt respond or data was transmitted unsuccessfully then break
//
//				buff += 512; //increment the buffer pointer to the starting address of the next block to be written
//			} while (--count); //decrement the total number of blocks to be written by 1
//
//			/* STOP_TRAN token */
//			SD_TxDataBlock(0, 0xFD);//This is specific to SD cards only ,
//									//For SD CARDS a multiple block write must be terminated by a STOP TRANSACTION token
//			if(!SD_SendCmd(SEND_STATUS, 0) == 0) //Check status of SDCARD if response = 0x00 then it successfully transmitted the STOP TRAN TOKEN
//
//			{
//				count = 1; //if the response is other than 0x00 then an error occured
//			}
//		}
//	}
//
//	/* Idle */
//	CS_Disable();
//	SPI_RxByte();
//
//	return count ? RES_ERROR : RES_OK;
}

DRESULT SD_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count){

//	Common_Printf("pdrv = %d , sector = %d , count = %d \r\n" , pdrv, sector, count);
//	/* pdrv should be 0 */
//	if (pdrv || !count){
//		return RES_PARERR;
//	}
//
//	/* no disk */
//	if (sdcd.mStat & STA_NOINIT){
//		return RES_NOTRDY;
//	}
//
//	/* convert to byte address */
//	if (sdcd.mType == SD_V2SC){
//		sector *= 512;
//	}
//
//	CS_Enable();
//
//	if (count == 1)
//	{
//		/* READ_SINGLE_BLOCK */
//		if ((SD_SendCmd(READ_SINGLE_BLOCK, sector) == 0) && SD_RxDataBlock(buff, 512)){
//			count = 0;
//		}
//	}
//	else
//	{
//		/* READ_MULTIPLE_BLOCK */
//		if (SD_SendCmd(READ_MULTIPLE_BLOCK, sector) == 0)
//		{
//			do {
//				if (!SD_RxDataBlock(buff, 512)){
//					break;
//				}
//				buff += 512;
//			} while (--count);
//
//			/* STOP_TRANSMISSION */
//			SD_SendCmd(STOP_TRANSMISSION, 0);
//		}
//	}
//
//	/* Idle */
//	CS_Disable();
//	SPI_RxByte();
//
//	return count ? RES_ERROR : RES_OK;
}


DRESULT SD_disk_ioctl(BYTE drv, BYTE ctrl, void *buff){
//	DRESULT res;
//	uint8_t n, csd[16], *ptr = buff;
//	WORD csize;
//
//	if (drv){
//		return RES_PARERR;
//	}
//
//	res = RES_ERROR;
//
//	if (ctrl == CTRL_POWER)
//	{
//		switch (*ptr)
//		{
//		case 0:
//			SD_PowerOff();		/* Power Off */
//			res = RES_OK;
//			break;
//		case 1:
//			SD_PowerOn();		/* Power On */
//			res = RES_OK;
//			break;
//		case 2:
//			*(ptr + 1) = SD_CheckPower();
//			res = RES_OK;		/* Power Check */
//			break;
//		default:
//			res = RES_PARERR;
//		}
//	}
//	else
//	{
//		if (sdcd.mStat & STA_NOINIT){
//			return RES_NOTRDY;
//		}
//
//		CS_Enable();
//
//		switch (ctrl)
//		{
//		case GET_SECTOR_COUNT:
//			/* SEND_CSD */
//			if ((SD_SendCmd(SEND_CSD, 0) == 0) && SD_RxDataBlock(csd, 16))
//			{
//				if ((csd[0] >> 6) == 1)
//				{
//					/* SDC V2 */
//					csize = csd[9] + ((WORD) csd[8] << 8) + 1; //Get the first 16 most signifcant bytes of the CSD register
//					*(DWORD*) buff = (DWORD) csize << 10;		// Bytes stored in csd[8] csd[9] form the MSB and LSB respectively of the of the device's size C_SIZE
//				}												// C_SIZE gives the device's size in terms of Blocks where 1 Unit of C_SIZE equals 1024 Block
//																// Each Block is 512 bytes
//				else											// multiply C_SIZE by 1024 to get the total number of blocks(sectors)
//				{
//					/* MMC or SDC V1 */
//					n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
//					csize = (csd[8] >> 6) + ((WORD) csd[7] << 2) + ((WORD) (csd[6] & 3) << 10) + 1;
//					*(DWORD*) buff = (DWORD) csize << (n - 9);
//				}
//				res = RES_OK;
//			}
//			break;
//		case GET_SECTOR_SIZE:
//			*(WORD*) buff = 512;
//			res = RES_OK;
//			break;
//		case CTRL_SYNC: //Checks if SDcard is busy or not
//			if (SD_ReadyWait() == 0xFF) res = RES_OK;
//			break;
//		case MMC_GET_CSD: //Get the first 16 most signifcant bytes of the CSD register
//			/* SEND_CSD */
//			if (SD_SendCmd(SEND_CSD,0) == 0 && SD_RxDataBlock(ptr, 16)) res = RES_OK;
//			break;
//		case MMC_GET_CID: //Get the first 16 most signifcant bytes of the CID register
//			/* SEND_CID */
//			if (SD_SendCmd(SEND_CID, 0) == 0 && SD_RxDataBlock(ptr, 16)) res = RES_OK;
//			break;
//		case MMC_GET_OCR:
//			/* READ_OCR */
//			if (SD_SendCmd(READ_OCR, 0) == 0)
//			{
//				for (n = 0; n < 4; n++)
//				{
//					*ptr++ = SPI_RxByte();
//				}
//				res = RES_OK;
//			}
//		default:
//			res = RES_PARERR;
//		}
//
//		CS_Disable();
//		SPI_RxByte();
//	}
//
//	return res;
}

DSTATUS SD_disk_status (BYTE pdrv){
	return sdcd.mStat;
}

