#ifndef __SDCARD_H
#define __SDCARD_H


typedef enum {
	SDC_CMD_GO_IDLE_STATE        = 0x00,  // CMD0: Resets the card and puts it in idle state.
	SDC_CMD_SEND_OP_COND         = 0x01,  // CMD1: Sends operation condition for SD cards.
	SDC_CMD_ALL_SEND_CID         = 0x02,  // CMD2: Sends the card identification number (CID).
	SDC_CMD_SEND_CSD             = 0x03,  // CMD3: Sends card-specific data (CSD).
	SDC_CMD_STOP_TRANSMISSION    = 0x05,  // CMD5: Stops data transmission.
	SDC_CMD_SWITCH_FUNC          = 0x06,  // CMD6: Switches function on the card.
	SDC_CMD_SELECT_CARD          = 0x07,  // CMD7: Selects the card for operation.
	SDC_CMD_SEND_IF_COND         = 0x08,  // CMD8: Sends interface condition.
	SDC_CMD_READ_DAT_UNTIL_STOP  = 0x0B,  // CMD11: Read data until the stop command.
	SDC_CMD_STOP_TRANSMISSION_2  = 0x0C,  // CMD12: Stops data transmission.
	SDC_CMD_SEND_STATUS          = 0x0D,  // CMD13: Sends status of the selected card.
	SDC_CMD_SET_BLOCKLEN         = 0x10,  // CMD16: Sets the block length for data transfers.
	SDC_CMD_READ_SINGLE_BLOCK    = 0x11,  // CMD17: Reads a single block of data.
	SDC_CMD_READ_MULTIPLE_BLOCK  = 0x12,  // CMD18: Reads multiple blocks of data.
	SDC_CMD_SET_BLOCK_COUNT      = 0x17,  // CMD23: Sets the number of blocks for multiple block transfer.
	SDC_CMD_WRITE_BLOCK          = 0x18,  // CMD24: Writes a single block of data.
	SDC_CMD_WRITE_MULTIPLE_BLOCK = 0x19,  // CMD25: Writes multiple blocks of data.
	SDC_CMD_ERASE_WR_BLK_START   = 0x20,  // CMD32: Sets the starting block for erase.
	SDC_CMD_ERASE_WR_BLK_END     = 0x21,  // CMD33: Sets the ending block for erase.
	SDC_CMD_ERASE                = 0x26,  // CMD38: Erases the previously selected blocks.
	SDC_CMD_APP_CMD              = 0x37,  // CMD55: Indicates that the next command is an application command.
	SDC_CMD_GEN_CMD              = 0x38,  // CMD56: General command for application-specific functions.
	SDC_CMD_READ_OCR             = 0x3A   // CMD58: Reads the card's operating condition register (OCR).
} SD_Card_CMD;

typedef enum {
    SDC_UNKNOWN           ,  // Unknown type or not recognized.
    SDC_STANDARD          ,  // Standard SD card (up to 2GB).
    SDC_HIGH_CAPACITY     ,  // SDHC: High Capacity (4GB to 32GB).
    SDC_EXTENDED_CAPACITY ,  // SDXC: Extended Capacity (32GB to 2TB).
    SDC_ULTRA_CAPACITY    ,  // SDUC: Ultra Capacity (up to 128TB).
    SDC_MMC               ,  // MultiMediaCard (MMC) type.
    SDC_MMC_PLUS          ,  // MMC Plus.
    SDC_MMC_HS            ,  // MMC High Speed.
    SDC_MMC_MOBILE        ,  // MMC Mobile.
    SDC_PRO               ,  // SD PRO: Professional grade cards.
    SDC_MICRO             ,  // Micro SD card (all types).
    SDC_COMPACT_FLASH        // CompactFlash (CF) type.
} SD_Card_Type;


#define SDCD_SUCCESS       0x0000
#define SDCD_HAL_FAIL      0x0001
#define SDCD_HAL_TIMEOUT   0x0002
#define SDCD_CMD_FAIL      0x0004
#define SDCD_CMD_TIMEOUT   0x0008
#define SDCD_READY_TIMEOUT 0x0010

#define SDCD_BLOCK_LEN 512u

typedef uint16_t _sdcd_err;

void SDCD_Init (SPI_HandleTypeDef* spi_d ,  GPIO_HandleTypeDef* gpiod );

/* Fatfs Interface Functions */
DSTATUS SD_disk_init(BYTE drv);
DSTATUS SD_disk_status (BYTE pdrv);
DRESULT SD_disk_read (BYTE pdrv, BYTE* buff, DWORD sector, UINT count);
DRESULT SD_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count);
DRESULT SD_disk_ioctl (BYTE pdrv, BYTE cmd, void* buff);

#endif
