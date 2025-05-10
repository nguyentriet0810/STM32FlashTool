#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include "stm32f10x.h"
#include "stddef.h"

typedef enum {
	// da sua lai
    RING_FAIL = 0,
    RING_OK = 1,
} RINGBUFF_STT_t;

typedef struct {
    volatile uint32_t head;
    volatile uint32_t tail;
    uint32_t size;
    volatile uint8_t *ptrRingBuff;
		volatile uint8_t full;
} RINGBUFF_TypeDef;

RINGBUFF_STT_t RINGBUFF_Init(RINGBUFF_TypeDef *ringBuff, uint8_t *ptr_ringBuff, uint32_t max_size);
RINGBUFF_STT_t RINGBUFF_Put(RINGBUFF_TypeDef *ringBuff, uint8_t value);
RINGBUFF_STT_t RINGBUFF_Get(RINGBUFF_TypeDef *ringBuff, uint8_t *value);
//RINGBUFF_STT_t RINGBUFF_GetTwo(RINGBUFF_TypeDef *ringBuff, uint8_t *val1, uint8_t *val2);

#endif /*__RING_BUFFER__*/
