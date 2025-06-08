/*
 * file_sys.c
 *
 *  Created on: Apr 20, 2025
 *      Author: rumbl
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "file_sys.h"
#include "fatfs.h"
#include "common.h"
#include "opts.h"

#define FS_FORCE_MOUNT 1u
#define FS_LABEL_SIZE 128u
#define FS_CONFIG_MAX_BUFF_SIZE 512u

struct {

	struct {
		FATFS ffs;
		bool v_mount;
		bool v_label;
		TCHAR label[FS_LABEL_SIZE];
		DWORD volumeSerialNumber;
	}drive;

	struct {
		FIL file;
		bool fValid;
		char fBuffer[FS_CONFIG_MAX_BUFF_SIZE];
		uint32_t fBufferLength;
	}json;

} fsys ;


static void Fsys_PrintMeta ( void ){
#if defined(_OPTS_DEBUG_EN) && _OPTS_DEBUG_EN == true

	if ( fsys.drive.v_mount == true ){
		Common_Printf("SDC Detected : \r\n");
		Common_Printf("fs_id : %d\r\n" , fsys.drive.ffs.id);
		Common_Printf("fs_type : %d\r\n" , fsys.drive.ffs.fs_type);
		Common_Printf("fs_drv : %d\r\n" , fsys.drive.ffs.drv);
		Common_Printf("fs_fsize : %d\r\n" , fsys.drive.ffs.fsize);
		Common_Printf("fs_csize : %d\r\n" , fsys.drive.ffs.csize);
		Common_Printf("fs_free_clust : %d\r\n" , fsys.drive.ffs.free_clust);
	}

	if ( fsys.drive.v_label == true ){
		Common_Printf("fs_label : %s\r\n" , &fsys.drive.label[0u]);
		Common_Printf("fs_serialNumber : %d\r\n" , fsys.drive.volumeSerialNumber);
	}
#endif
}

static _fsys_err Fsys_GetMeta ( void ){

	FRESULT res = f_getlabel("",&fsys.drive.label[0u], &fsys.drive.volumeSerialNumber);

	if ( res != FR_OK ){
		return FSYS_FAIL;
	}

	return FSYS_SUCCESS;
}

extern _fsys_err Fsys_Init( void ){

	uint32_t tp = lw_Sys_Get_Ticks();
	FRESULT res = f_mount(&fsys.drive.ffs, "0:/", FS_FORCE_MOUNT);
	uint32_t ta = lw_Sys_Get_Ticks();

	Common_Printf("f_mount %d ms" , ta-tp);

	if ( res != FR_OK ){
		fsys.drive.v_mount = false;
		return FSYS_FAIL;
	}

	fsys.drive.v_mount = true;

	if ( Fsys_GetMeta() != FSYS_SUCCESS ){
		fsys.drive.v_label = false;
	}

	fsys.drive.v_label = true;

#if defined(_OPTS_DEBUG_EN) && _OPTS_DEBUG_EN == true
	Fsys_PrintMeta();
#endif

	return FSYS_SUCCESS;
}

extern _fsys_err Fsys_GetConfig ( char** sptr , uint32_t* lptr ) {
	if ( fsys.json.fValid != true ||
		 fsys.json.fBufferLength == 0u ){
		return FSYS_INVALID_FILE;
	}
	*sptr = (char*)&fsys.json.fBuffer[0u];
	*lptr = fsys.json.fBufferLength;

	return FSYS_SUCCESS;
}

extern _fsys_err Fsys_LoadConfig ( void ){

	UINT rb = 0U;
	FRESULT res;
	DIR dir;
	FILINFO fno;
	TCHAR arr2[256u];

	fno.lfname = arr2;
	fno.lfsize = 256u;

	res = f_opendir(&dir,"slug");

	if ( res != FR_OK ){
		Common_Printf("f_opendir failed %d \r\n" , res);
		return FSYS_FAIL;
	}

	res = f_readdir(&dir,&fno);

	if ( res != FR_OK ){
		Common_Printf("f_readdir failed %d \r\n" , res);
		return FSYS_FAIL;
	}

	Common_Printf("file_name : %s\r\n" , fno.lfname);

#if _USE_LFN != 0
    res = f_open(&fsys.json.file,"0:/slug/config.json",FA_READ);
#else
    res = f_open(&fsys.json.file,"0:/slug/CONFIG~1.JSO",FA_READ);
#endif
	if ( res != FR_OK ){
		Common_Printf("f_open failed %d \r\n" , res);
		return FSYS_FAIL;
	}

	res = f_read(&fsys.json.file,&fsys.json.fBuffer[0u],FS_CONFIG_MAX_BUFF_SIZE,&rb);

	if ( res != FR_OK ){
		Common_Printf("f_read failed %d\r\n" , res);
		return FSYS_FAIL;
	}

	fsys.json.fBufferLength = rb;
	fsys.json.fValid = true;

	Common_Printf("FileSize : %d \r\n  File : %s \r\n", rb , fsys.json.fBuffer);
	Common_Printf("Strlen : %d \r\n" , strlen(&fsys.json.fBuffer[0u]));

	res = f_close(&fsys.json.file);

	if ( res != FR_OK ){
		Common_Printf("f_close failed\r\n");
		return FSYS_FAIL;
	}

	Common_Printf("Loading Successful\r\n");

	return FSYS_SUCCESS;
}

