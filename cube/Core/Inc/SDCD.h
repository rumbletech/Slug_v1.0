#ifndef __SDCARD_H
#define __SDCARD_H

void SDCD_Init (SPI_HandleTypeDef* spi_d ,  GPIO_HandleTypeDef* gpiod );

/* Fatfs Interface Functions */
DSTATUS SD_disk_init(BYTE drv);
DSTATUS SD_disk_status (BYTE pdrv);
DRESULT SD_disk_read (BYTE pdrv, BYTE* buff, DWORD sector, UINT count);
DRESULT SD_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count);
DRESULT SD_disk_ioctl (BYTE pdrv, BYTE cmd, void* buff);

#endif
