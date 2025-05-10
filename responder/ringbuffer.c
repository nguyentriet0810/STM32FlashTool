#include "ringbuffer.h"
//Khoi tao buffer
RINGBUFF_STT_t RINGBUFF_Init(RINGBUFF_TypeDef *ringBuff, uint8_t *ptr_ringBuff, uint32_t max_size) {
    if (!ringBuff || !ptr_ringBuff || max_size < 2) {
        return RING_FAIL;
    }
    ringBuff->ptrRingBuff = ptr_ringBuff;
    ringBuff->head = 0; 
    ringBuff->tail = 0;
    ringBuff->size = max_size;
    ringBuff->full = 0; //bao buffer day
    return RING_OK;
}

//Ghi du lieu vao buffer
RINGBUFF_STT_t RINGBUFF_Put(RINGBUFF_TypeDef *ringBuff, uint8_t value) {
    if (!ringBuff || !ringBuff->ptrRingBuff) return RING_FAIL;
		// neu buffer day
    if (ringBuff->full) {
        return RING_FAIL;  
    }
		// ghi du lieu vao buffer
    ringBuff->ptrRingBuff[ringBuff->head] = value;
    ringBuff->head = (ringBuff->head + 1) % ringBuff->size; //tro den o tiep theo
		//neu trung voi duoi thi bao buffer day
    if (ringBuff->head == ringBuff->tail) {
        ringBuff->full = 1;  
				return RING_FAIL;
    }
    return RING_OK;
}

// ham lay 1 ky tu tu buffer
RINGBUFF_STT_t RINGBUFF_Get(RINGBUFF_TypeDef *ringBuff, uint8_t *value) {
    if (!ringBuff || !ringBuff->ptrRingBuff || !value) return RING_FAIL;

    if (ringBuff->head == ringBuff->tail && !ringBuff->full) {
        return RING_FAIL; // Buffer rong
    }
    *value = ringBuff->ptrRingBuff[ringBuff->tail];
    ringBuff->tail = (ringBuff->tail + 1) % ringBuff->size;
    ringBuff->full = 0;  //buffer ko con day nua

    return RING_OK; // tra ve get thanh cong
}

