/*
 * diskt.h
 *
 *  Created on: Jun 21, 2025
 *      Author: rumbl
 */

#ifndef INC_DISKT_H_
#define INC_DISKT_H_


#include "common.h"
#include "fatfs.h"
#include "FreeRTOS.h"
#include "queue.h"


enum e_diskt_request {

	e_diskt_request_mount,
	e_diskt_request_get_label,
	e_diskt_open_file,
	e_diskt_read_file,
	e_diskt_write_file,
	e_diskt_close_file,
	e_diskt_flush_file,
	e_diskt_request_length,
};

union diskt_io_request_s {

	struct diskt_request_write_s{

		FIL* fptr;
		void* dptr;
		uint32_t btw;

	}write_request;

	struct diskt_request_open_s{
		FIL* fptr;
		char* path;
		uint32_t opts;
	}open_request;

	struct diskt_request_flush_s{
		FIL* fptr;
	}flush_request;

	struct diskt_request_close_s{
		FIL* fptr;
	}close_request;

	struct diskt_request_read_s{
		FIL* fptr;
		uint8_t* readBuffer;
		uint32_t maxSize;
	}read_request;

};

struct diskt_request_s {
	enum e_diskt_request reqType;
	union diskt_io_request_s reqArg;
};

struct diskt_response_s {
	ProcStatus_t proc_rc ;
	FRESULT ffs_rc;
	uint32_t ffs_bc;
};


extern void DISKT_Init ( QueueHandle_t qRequest , QueueHandle_t qResponse );
extern ProcStatus_t DISKT_PostRequest ( struct diskt_request_s request );
extern ProcStatus_t DISKT_GetResponse ( struct diskt_response_s* response );
extern void DISKT_Process ( void );

#endif /* INC_DISKT_H_ */
