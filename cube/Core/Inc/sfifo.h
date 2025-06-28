/*
 * sfifo.h
 *
 *  Created on: Jun 13, 2025
 *      Author: rumbl
 */

#ifndef INC_SFIFO_H_
#define INC_SFIFO_H_


#include <stdint.h>
#include <stdbool.h>

struct ppbf_s {
    uint8_t* pingbuffer;
    uint32_t pingBufferLength;
    uint32_t pingBufferSize;
    uint8_t* pongbuffer;
    uint32_t pongBufferLength;
    uint32_t pongBufferSize;
    uint8_t  ctx;
    uint8_t  err;
};

/* Initializes the buffers and data struct */
extern void ppbf_init ( struct ppbf_s* pp , uint8_t* pingBuffer , uint8_t* pongBuffer , uint32_t pingBufferSize , uint32_t pongBufferSize );
/* Resets all the errors and number of data filled in each buffer */
extern void ppbf_reset( struct ppbf_s* pp );
/* Gets the error state */
extern uint8_t ppbf_get_err( struct ppbf_s* pp );
/* Gets the current active read buffer */
extern uint8_t* ppbf_get_read_ptr( struct ppbf_s* pp );
/* Gets the current active read buffer length */
extern uint32_t ppbf_get_read_len( struct ppbf_s* pp );
/* Gets the current active write buffer */
extern uint8_t* ppbf_get_write_ptr( struct ppbf_s* pp );
/* Gets the current active write buffer length */
extern uint32_t ppbf_get_write_len( struct ppbf_s* pp );
/* Swaps the ping pong buffers -> needs to be called before reading */
extern void ppbf_switch( struct ppbf_s* pp );
/* Writes to the current active buffer */
extern void ppbf_write( struct ppbf_s* pp , uint8_t* data , uint32_t len );


#endif /* INC_SFIFO_H_ */
