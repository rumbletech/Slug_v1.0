#include "common.h"
#include "fatfs.h"
#include "GPIO.h"
#include <stdbool.h>
#include "SDCD.h"

#define SPI_TIMEOUT 100

#define _SDCD_ERRH_ 0x02

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
 }

static uint8_t get_CRC(uint8_t cmd_idx){
	if(cmd_idx == GO_IDLE_STATE ) return 0x95;
	else if (cmd_idx == SEND_IF_COND) return 0x87;
	else return 1;

}

static inline void CS_Enable(){
	HAL_GPIO_WritePin(sdcd.spi_cs->port, sdcd.spi_cs->pinNum, GPIO_PIN_RESET);
}

static inline void CS_Disable(){
	HAL_GPIO_WritePin(sdcd.spi_cs->port, sdcd.spi_cs->pinNum, GPIO_PIN_SET);
}

static void SPI_TxBuffer(uint8_t *buffer, uint16_t len){
	while(!__HAL_SPI_GET_FLAG(sdcd.spi_d, SPI_FLAG_TXE));
	HAL_SPI_Transmit(sdcd.spi_d , buffer, len, SPI_TIMEOUT);
}

static void SPI_TxByte(uint8_t data){
	while(!__HAL_SPI_GET_FLAG(sdcd.spi_d, SPI_FLAG_TXE));
	HAL_SPI_Transmit(sdcd.spi_d, &data, 1, SPI_TIMEOUT);
}

static uint8_t SPI_RxByte(){
	uint8_t dummy, data;
	dummy = 0xFF;
	while(!__HAL_SPI_GET_FLAG(sdcd.spi_d, SPI_FLAG_TXE));
	HAL_SPI_TransmitReceive(sdcd.spi_d, &dummy, &data, 1, SPI_TIMEOUT);
	return data;
}

static void SPI_RxBytePtr(uint8_t *buff){
	*buff = SPI_RxByte();
}

/* power off */
static inline void SD_PowerOff(void)
{
	sdcd.pwrf = 0;
}

static uint8_t SD_ReadyWait(void){
	uint8_t res;

	/* timeout 500ms */
	uint16_t timeOut = 500;

	uint32_t elapsedTicks;
	uint32_t currentTicks = HAL_GetTick();

	/* if SD goes ready, receives 0xFF */
	do {
		res = SPI_RxByte();
		elapsedTicks = HAL_GetTick()-currentTicks;
	} while ( (res != 0xFF) &&
			  (elapsedTicks < timeOut));

	return res;
}

static uint8_t SD_SendCmd(enum memd_cmd cmd , uint32_t arg ){
	uint8_t res = 0;
	if (SD_ReadyWait() != 0xFF) return false;
	uint8_t command[6];
	command[5] = 0b01000000 | (uint8_t)cmd;   //Command Frame | 0 | 1 |  CMD_IDX  |  CMD_ARG  |  CRC  | 1 |
												  //			   47  46  45       40 39        8 7     1  0
	command[4] = (uint8_t)(arg >> 24) ;
	command[3] = (uint8_t)(arg >> 16) ;
	command[2] = (uint8_t)(arg >> 8) ;
	command[1] = (uint8_t)(arg) ;
	command[0] = get_CRC(cmd);

		SPI_TxBuffer(command,6);

	//Discard the byte received right after a STOP_TRANSMISSION command prior to receiving the response
	if(cmd == STOP_TRANSMISSION) SPI_RxByte();

	/* receive response */
	uint8_t n = 10;

	do {
		res = SPI_RxByte();
	} while ((res & 0x80) && --n); //if R1 response equals 0b1000_0000 then that means that the SDCARD is busy as MSB of R1 response is busy flag

	return res;

}

static uint8_t SD_CheckPower(void)
{
	return sdcd.pwrf;
}

static bool SD_PowerOn(){

	CS_Disable();
	for(int i = 0 ; i < 10 ; i++){
		SPI_TxByte((uint8_t)0xff);
	}
	CS_Enable();
	uint8_t response = SD_SendCmd(GO_IDLE_STATE,0);
	CS_Disable();
	if(response == 0x01){
		sdcd.pwrf = true;
		SPI_TxByte(0xff);
	}
	else {
		sdcd.pwrf = false ;
	}
	return sdcd.pwrf;
}

DSTATUS SD_disk_init(BYTE drv)
{
	uint32_t currentTicks , elapsedTicks , timeOut;
	Common_Printf("SD_disk_init\r\n");
	/* single drive, drv should be 0 */
	if(drv){
		Common_Printf("STA_NOINIT\r\n");
		return STA_NOINIT;
	}

		/* no disk */
	if(sdcd.mStat & STA_NODISK){
		Common_Printf("STA_NODISK\r\n");
		return sdcd.mStat;
	}
	
	//goes through the initialization sequence for the SD card
	if(SD_PowerOn() == true){
		Common_Printf("SD_PowerOn\r\n");
		sdcd.mType = UNKNOWN_CARD ;
		return STA_NOINIT;
	}

	uint8_t OCR[4];
	CS_Enable();
	if(SD_SendCmd(SEND_IF_COND,0x000001AA) == 1){
		Common_Printf("SD_SendCmd , SEND_IF_COND , Success\r\n");
		for(int i = 3 ; i >= 0 ; i--){
			OCR[i] = SPI_RxByte();
		}
		if(OCR[1] == 0x01 && OCR[0] == 0xAA) {

			currentTicks = HAL_GetTick();
			timeOut = 200;

			do {
				elapsedTicks = HAL_GetTick()-currentTicks;
				if(SD_SendCmd(APP_CMD,0) <= 0x01){
					if(SD_SendCmd(APP_SEND_OP_COND,0x40000000) <= 0x01){
						break;
					}
				}

			}
			while(elapsedTicks < timeOut); //Send CMD55 and CMD41 until you get a response or until a timeout occurs

			if(elapsedTicks < timeOut){ //if timeout didnt occur then and CMD55 , CMD41 were recieved successfully then send CMD58
				if(SD_SendCmd(READ_OCR,0) == 1) {//Send CMD58
					for(int i = 3 ; i >= 0 ; i--){
								OCR[i] = SPI_RxByte(); //Read and store OCR register contents
							}
					if(OCR[3]==0x40)
						sdcd.mType = SD_V2HC;//If CCS bit is set then type is SDV2HC
					else
						sdcd.mType = SD_V2SC; //If CCS bit is set then type is SDV2SC
				}
			}
			else{
				sdcd.mType = UNKNOWN_CARD;
				return STA_NOINIT;
			}
		}

	}
	else
			{
				/* SDC V1 or MMC */
				sdcd.mType = (SD_SendCmd(APP_CMD, 0) <= 1 && SD_SendCmd(APP_SEND_OP_COND, 0) <= 1) ? SD_V1 : MMC_V3;

				currentTicks = HAL_GetTick();
				timeOut = 200;

				do
				{
					elapsedTicks = HAL_GetTick()-currentTicks;
					if (sdcd.mType == SD_V1)
					{
						if (SD_SendCmd(APP_CMD, 0) <= 1 && SD_SendCmd(APP_SEND_OP_COND, 0) == 0) break; /* ACMD41 */
					}
					else
					{
						if (SD_SendCmd(SEND_OP_COND, 0) == 0) break; /* CMD1 */
					}

				} while (elapsedTicks < timeOut);


				if ( elapsedTicks >= timeOut ) {
					sdcd.mType = UNKNOWN_CARD;
				}
			}
	if(sdcd.mType  == SD_V1 ||
	   sdcd.mType  == SD_V2SC ||
	   sdcd.mType  == MMC_V3){
			if(SD_SendCmd(SET_BLOCKLEN, 512) == 0){
			}
			else {
				sdcd.pwrf = false;
				sdcd.mType  = UNKNOWN_CARD;
				
			}

	}
	CS_Disable();
	/* Clear STA_NOINIT */
	if (sdcd.mType )
	{
		sdcd.mStat &= ~STA_NOINIT;
	}
	else
	{
	/* Initialization failed */
		sdcd.pwrf = false ;
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
	uint8_t token;

	/* timeout 200ms */
	uint16_t timeOut = 200;

	uint32_t elapsedTicks;
	uint32_t currentTicks = HAL_GetTick();

	/* loop until receive a response or timeout */
	do {
		token = SPI_RxByte();
		elapsedTicks = HAL_GetTick()-currentTicks;
	} while((token != 0xFE) &&
			(elapsedTicks < timeOut));


	if ( elapsedTicks >= timeOut ){
		return false;
	}

	if(token != 0xFE ){
		return false;
	}

	/* receive data */
	//after receiveing token start receiving data block
	do {
		SPI_RxBytePtr(buff++);
	} while(len--);

	/* discard CRC */
	SPI_RxByte();
	SPI_RxByte();

	return true;
}

DRESULT SD_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count){
	/* pdrv should be 0 */
	if (pdrv || !count) return RES_PARERR;

	/* no disk */
	if (sdcd.mStat & STA_NOINIT) return RES_NOTRDY;

	/* write protection */
	if (sdcd.mStat & STA_PROTECT) return RES_WRPRT;

	/* convert to byte address */
	if (sdcd.mType == SD_V2SC) sector *= 512;

	CS_Enable();

	if (count == 1)
	{
		/* WRITE_BLOCK */
		if ((SD_SendCmd(WRITE_BLOCK, sector) == 0)) //Send CMD 24 WRITE_SINGLE_BLOCK
			if(SD_TxDataBlock(buff, 0xFE)) //Send datablock stored in the buffer , with CMD 24 token 0xFE
			count = 0;
	}
	else
	{
		/* WRITE_MULTIPLE_BLOCK */
		if (sdcd.mType == SD_V1)
		{
			SD_SendCmd(SET_BLOCK_COUNT, 0);   // For MMC, the number of blocks to write must be pre-defined by CMD23(SET_BLOCK_COUNT) prior to CMD25(WRITE_MULTIPLE_BLOCKS)
									//	and the write transaction is terminated at last data block.
			SD_SendCmd(WRITE_MULTIPLE_BLOCK, count); /* ACMD23 */
		}

		if (SD_SendCmd(WRITE_MULTIPLE_BLOCK, sector) == 0) //send CMD 25 with starting sector address
		{
			do {
				if(!SD_TxDataBlock(buff, 0xFC)) break; // if SDCARD timesout and doesnt respond or data was transmitted unsuccessfully then break

				buff += 512; //increment the buffer pointer to the starting address of the next block to be written
			} while (--count); //decrement the total number of blocks to be written by 1

			/* STOP_TRAN token */
			SD_TxDataBlock(0, 0xFD);//This is specific to SD cards only ,
									//For SD CARDS a multiple block write must be terminated by a STOP TRANSACTION token
			if(!SD_SendCmd(SEND_STATUS, 0) == 0) //Check status of SDCARD if response = 0x00 then it successfully transmitted the STOP TRAN TOKEN

			{
				count = 1; //if the response is other than 0x00 then an error occured
			}
		}
	}

	/* Idle */
	CS_Disable();
	SPI_RxByte();

	return count ? RES_ERROR : RES_OK;
}

DRESULT SD_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count){
	/* pdrv should be 0 */
	if (pdrv || !count) return RES_PARERR;

	/* no disk */
	if (sdcd.mStat & STA_NOINIT) return RES_NOTRDY;

	/* convert to byte address */
	if (sdcd.mType == SD_V2SC){
		sector *= 512;
	}

	CS_Enable();

	if (count == 1)
	{
		/* READ_SINGLE_BLOCK */
		if ((SD_SendCmd(READ_SINGLE_BLOCK, sector) == 0) && SD_RxDataBlock(buff, 512)) count = 0;
	}
	else
	{
		/* READ_MULTIPLE_BLOCK */
		if (SD_SendCmd(READ_MULTIPLE_BLOCK, sector) == 0)
		{
			do {
				if (!SD_RxDataBlock(buff, 512)) break;
				buff += 512;
			} while (--count);

			/* STOP_TRANSMISSION */
			SD_SendCmd(STOP_TRANSMISSION, 0);
		}
	}

	/* Idle */
	CS_Disable();
	SPI_RxByte();

	return count ? RES_ERROR : RES_OK;
}


DRESULT SD_disk_ioctl(BYTE drv, BYTE ctrl, void *buff){
	DRESULT res;
	uint8_t n, csd[16], *ptr = buff;
	WORD csize;

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

		CS_Enable();

		switch (ctrl)
		{
		case GET_SECTOR_COUNT:
			/* SEND_CSD */
			if ((SD_SendCmd(SEND_CSD, 0) == 0) && SD_RxDataBlock(csd, 16))
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
			break;
		case GET_SECTOR_SIZE:
			*(WORD*) buff = 512;
			res = RES_OK;
			break;
		case CTRL_SYNC: //Checks if SDcard is busy or not
			if (SD_ReadyWait() == 0xFF) res = RES_OK;
			break;
		case MMC_GET_CSD: //Get the first 16 most signifcant bytes of the CSD register
			/* SEND_CSD */
			if (SD_SendCmd(SEND_CSD,0) == 0 && SD_RxDataBlock(ptr, 16)) res = RES_OK;
			break;
		case MMC_GET_CID: //Get the first 16 most signifcant bytes of the CID register
			/* SEND_CID */
			if (SD_SendCmd(SEND_CID, 0) == 0 && SD_RxDataBlock(ptr, 16)) res = RES_OK;
			break;
		case MMC_GET_OCR:
			/* READ_OCR */
			if (SD_SendCmd(READ_OCR, 0) == 0)
			{
				for (n = 0; n < 4; n++)
				{
					*ptr++ = SPI_RxByte();
				}
				res = RES_OK;
			}
		default:
			res = RES_PARERR;
		}

		CS_Disable();
		SPI_RxByte();
	}

	return res;
}

DSTATUS SD_disk_status (BYTE pdrv){
	return sdcd.mStat;
}

