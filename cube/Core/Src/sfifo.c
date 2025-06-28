/*
 * sfifo.c
 *
 *  Created on: Jun 13, 2025
 *      Author: rumbl
 */


#include "sfifo.h"
#include <string.h>

#define WR_PING_RD_PONG 0U
#define WR_PONG_RD_PING 1U

extern void ppbf_init ( struct ppbf_s* pp , uint8_t* pingBuffer , uint8_t* pongBuffer , uint32_t pingBufferSize , uint32_t pongBufferSize ){

	pp->err = 0u;
	pp->ctx = WR_PING_RD_PONG;
	pp->pingbuffer = pingBuffer;
	pp->pingBufferSize = pingBufferSize;
	pp->pingBufferLength = 0ul;

	pp->pongbuffer = pongBuffer;
	pp->pongBufferSize = pongBufferSize;
	pp->pongBufferLength = 0ul;
}

extern void ppbf_reset( struct ppbf_s* pp ){
	pp->err = 0u;
	pp->ctx = WR_PING_RD_PONG;
	pp->pingBufferLength = 0ul;
	pp->pongBufferLength = 0ul;
}

extern uint8_t ppbf_get_err( struct ppbf_s* pp ){
	return pp->err ;
}

extern uint8_t* ppbf_get_read_ptr( struct ppbf_s* pp ){
	if ( pp->ctx == WR_PING_RD_PONG ){
		return pp->pongbuffer;
	}
	else{
		return pp->pingbuffer;
	}
}

extern uint32_t ppbf_get_read_len( struct ppbf_s* pp ){
	if ( pp->ctx == WR_PING_RD_PONG ){
		return pp->pongBufferLength;
	}
	else{
		return pp->pingBufferLength;
	}
}

extern uint8_t* ppbf_get_write_ptr( struct ppbf_s* pp ){
	if ( pp->ctx == WR_PING_RD_PONG ){
		return pp->pingbuffer;
	}
	else{
		return pp->pongbuffer;
	}
}

extern uint32_t ppbf_get_write_len( struct ppbf_s* pp ){
	if ( pp->ctx == WR_PING_RD_PONG ){
		return pp->pingBufferLength;
	}
	else{
		return pp->pongBufferLength;
	}
}

extern void ppbf_switch( struct ppbf_s* pp ){
	if ( pp->ctx == WR_PING_RD_PONG ){
		pp->ctx = WR_PONG_RD_PING;
		pp->pongBufferLength = 0ul;
	}
	else{
		pp->ctx = WR_PING_RD_PONG;
		pp->pingBufferLength = 0ul;
	}
}

extern void ppbf_write( struct ppbf_s* pp , uint8_t* data , uint32_t len ){

	uint8_t* activeBuffer;
	uint32_t* activeBufferLength;
	uint32_t activeBufferSize;

	if ( pp->ctx == WR_PING_RD_PONG ){
		activeBuffer = pp->pingbuffer;
		activeBufferLength = &pp->pingBufferLength;
		activeBufferSize = pp->pingBufferSize;
	}
	else{
		activeBuffer = pp->pongbuffer;
		activeBufferLength = &pp->pongBufferLength;
		activeBufferSize = pp->pongBufferSize;
	}

	/* OverFlow */
	if ( (len + *activeBufferLength) > activeBufferSize ){
		pp->err = 1u;
		return;
	}

	memcpy((void*)(activeBuffer + (*activeBufferLength)) ,(void*)data ,len);
	*activeBufferLength = (*activeBufferLength) + len ;

	return;
}


