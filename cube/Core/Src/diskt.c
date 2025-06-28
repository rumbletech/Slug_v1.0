/*
 * diskt.c
 *
 *  Created on: Jun 21, 2025
 *      Author: rumbl
 */


#include "diskt.h"

#define FS_FORCE_MOUNT 1u
#define FS_LABEL_SIZE 128u

static QueueHandle_t RequestQueue;
static QueueHandle_t ResponseQueue;

struct diskt_data_s {

	FATFS ffs;
	TCHAR volumeLabel[FS_LABEL_SIZE];
	DWORD volumeSerialNumber;
};

static struct diskt_data_s diskt;

extern ProcStatus_t DISKT_PostRequest ( struct diskt_request_s request ){

	uint32_t ret;
	uint32_t qret = xQueueSend(RequestQueue, &request, 0u);

	if ( qret == pdPASS ){
		ret = E_OK;
	}
	else if( qret == errQUEUE_FULL ){
		ret = E_PENDING;
	}
	else{
		ret = E_NOT_OK;
	}

	return ret;

}

extern ProcStatus_t DISKT_GetResponse ( struct diskt_response_s* response ){

	uint32_t ret;
	uint32_t qret = xQueueReceive(ResponseQueue, response, 0u);

	if ( qret == pdPASS ){
		ret = E_OK;
	}
	else if ( qret == errQUEUE_EMPTY ){
		ret = E_PENDING;
	}
	else{
		ret = E_NOT_OK;
	}

	return ret;

}

static ProcStatus_t get_process_return ( FRESULT fres ){

	/* Interpret failures here */
	if ( fres != FR_OK ){
		return E_NOT_OK;
	}

	return E_OK;
}


static void handleReqeust( struct diskt_request_s request , struct diskt_response_s* response ){

	FRESULT fres = FR_DISK_ERR;
	uint32_t byteCount = 0u;

	switch(request.reqType){

		case e_diskt_request_mount:
		{
			Common_Printf("[DISKT_REQUEST_MOUNT]\r\n");
			fres = f_mount(&diskt.ffs, "0:/", FS_FORCE_MOUNT);

			if ( fres != FR_OK ){
				Common_Printf("[F]\r\n");
				break;
			}
			Common_Printf("[S]\r\n");
			Common_Printf("Mounting Successful\r\n");

			Common_Printf("fs_id : %d\r\n" , diskt.ffs.id);
			Common_Printf("fs_type : %d\r\n" , diskt.ffs.fs_type);
			Common_Printf("fs_drv : %d\r\n" , diskt.ffs.drv);
			Common_Printf("fs_fsize : %d\r\n" , diskt.ffs.fsize);
			Common_Printf("fs_csize : %d\r\n" , diskt.ffs.csize);
			Common_Printf("fs_free_clust : %d\r\n" , diskt.ffs.free_clust);
		}
		break;

		case e_diskt_request_get_label:
		{
			Common_Printf("[DISKT_REQUEST_LABEL]");
			fres = f_getlabel("",&diskt.volumeLabel[0u], &diskt.volumeSerialNumber);

			if ( fres != FR_OK ){
				Common_Printf("[F]\r\n");
				break;
			}

			Common_Printf("[S]\r\n");
			Common_Printf("fs_label : %s\r\n" , &diskt.volumeLabel[0u]);
			Common_Printf("fs_serialNumber : %d\r\n" , &diskt.volumeSerialNumber);

		}
		break;

		case e_diskt_open_file:
		{
			Common_Printf("[DISK_REQUEST_OPEN]");

			fres = f_open(request.reqArg.open_request.fptr,
						  request.reqArg.open_request.path,
						  request.reqArg.open_request.opts);

			if ( fres != FR_OK ){
				Common_Printf("[F]\r\n");
				break;
			}

			Common_Printf("[S]\r\n");
		}
		break;
		case e_diskt_read_file:
		{
			Common_Printf("[DISK_REQUEST_READ]");

			fres = f_read(request.reqArg.read_request.fptr,
						  request.reqArg.read_request.readBuffer,
						  request.reqArg.read_request.maxSize,
						  &byteCount);

			if ( fres != FR_OK ){
				Common_Printf("[F]\r\n");
				break;
			}
			Common_Printf("[S]\r\n");
		}
		break;

		case e_diskt_write_file:
		{
			Common_Printf("[DISK_REQUEST_WRITE]");

			fres = f_write(request.reqArg.write_request.fptr,
						   request.reqArg.write_request.dptr,
						   request.reqArg.write_request.btw,
						   &byteCount);

			if ( fres != FR_OK ){
				Common_Printf("[F]\r\n");
				break;
			}
			Common_Printf("[S]\r\n");
		}
		break;

		case e_diskt_flush_file:
		{
			Common_Printf("[DISK_REQUEST_FLUSH]");

			fres = f_sync(request.reqArg.flush_request.fptr);

			if ( fres != FR_OK ){
				Common_Printf("[F]\r\n");
				break;
			}
			Common_Printf("[S]\r\n");

		}
		break;

		case e_diskt_close_file:
		{
			Common_Printf("[DISK_REQUEST_CLOSE]");

			fres = f_close(request.reqArg.close_request.fptr);

			if ( fres != FR_OK ){
				Common_Printf("[F]\r\n");
				break;
			}
			Common_Printf("[S]\r\n");
		}
		break;
		default :
		{
			fres = FR_DISK_ERR;
			Common_Printf("[DISKT_UNKNOWN_REQUEST[F]]\r\n");
		}
		break;
	}

	Common_Printf("FRES[%d]\r\n",fres);

	response->proc_rc = get_process_return(fres);
	response->ffs_rc = fres;
	response->ffs_bc = byteCount;

}

extern void DISKT_Init ( QueueHandle_t qRequest , QueueHandle_t qResponse ){
	RequestQueue = qRequest;
	ResponseQueue = qResponse;
}


extern void DISKT_Process ( void ){

#define DISKT_PROC_STATE_RECEIVE_REQUEST 0U
#define DIKST_PROC_STATE_TRANSMIT_RESPONSE 1U

	struct diskt_request_s request;
	struct diskt_response_s response;
	static uint8_t state = DISKT_PROC_STATE_RECEIVE_REQUEST;
	uint32_t qRet = pdFAIL;

	if ( state == DISKT_PROC_STATE_RECEIVE_REQUEST ){
		uint32_t qRet =  xQueueReceive(RequestQueue, &request, portMAX_DELAY);
		if ( qRet == pdPASS ){
			handleReqeust(request,&response);
			state = DIKST_PROC_STATE_TRANSMIT_RESPONSE;
		}
	}

	if ( state == DIKST_PROC_STATE_TRANSMIT_RESPONSE ){
		qRet = xQueueSend(ResponseQueue, &response, portMAX_DELAY);
		if ( qRet == pdPASS ){
			state = DISKT_PROC_STATE_RECEIVE_REQUEST;
		}
	}
}
