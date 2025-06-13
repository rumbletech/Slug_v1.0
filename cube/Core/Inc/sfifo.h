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

struct fifo_s {
    uint8_t* buffer;
    uint16_t size;
    uint16_t head;
    uint16_t tail;
    uint16_t count;
};


extern void Fifo_Init( struct fifo_s * fr, uint8_t* bp , uint16_t sz);
extern void Fifo_Reset( struct fifo_s * fr );
extern uint16_t Fifo_GetCount(struct fifo_s * fr);
extern bool Fifo_isFull(struct fifo_s * fr);
extern bool Fifo_isEmpty(struct fifo_s * fr);
extern void Fifo_Push(struct fifo_s * fr , uint8_t data);
extern bool Fifo_Pop(struct fifo_s * fr , uint8_t *data);

#endif /* INC_SFIFO_H_ */
