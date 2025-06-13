/*
 * sfifo.c
 *
 *  Created on: Jun 13, 2025
 *      Author: rumbl
 */


#include "sfifo.h"

extern void Fifo_Init( struct fifo_s * fr, uint8_t* bp , uint16_t sz) {
	fr->head = 0u;
	fr->tail = 0u;
	fr->count = 0u;
	fr->buffer = bp;
	fr->size = sz;
}

extern void Fifo_Reset( struct fifo_s * fr ) {
	fr->head = 0u;
	fr->tail = 0u;
	fr->count = 0u;
}

extern uint16_t Fifo_GetCount(struct fifo_s * fr) {
    return fr->count;
}

extern bool Fifo_isFull(struct fifo_s * fr) {
    return fr->count >= fr->size;
}

extern bool Fifo_isEmpty(struct fifo_s * fr) {
    return fr->count == 0u;
}

extern void Fifo_Push(struct fifo_s * fr , uint8_t data) {
    fr->buffer[fr->head] = data;
    fr->head = (fr->head + 1) % fr->size;
    fr->count++;
}

extern bool Fifo_Pop(struct fifo_s * fr , uint8_t *data) {
    *data = fr->buffer[fr->tail];
    fr->tail = (fr->tail + 1) % fr->size;
    fr->count--;
}
