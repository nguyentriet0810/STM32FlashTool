#ifndef __FLASH_H__
#define __FLASH_H__

#include <stdint.h>
#include <stm32f10x.h>

void Flash_Erase(uint32_t addresspage);
void Flash_WriteWord(uint32_t address, uint32_t data);
void Flash_WriteNumByte(uint32_t address, uint8_t *data, int num);

#endif
