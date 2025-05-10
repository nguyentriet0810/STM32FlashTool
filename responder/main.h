#ifndef __MAIN_H__
#define __MAIN_H__

#include <stm32f10x.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ringbuffer.h"
#include "uart.h"
#include "flash.h"

#define SYSCLK 72000000UL 

//page 15
#define app_infor_start_address 0x08003C00U
#define app_infor_end_address   0x08003FFFU
//page 16
#define app_start_address 0x08004000U

#define START_BYTE 0xA5
#define STOP_BYTE  0x5A

typedef enum {
	MIN_STATE_IDLE  = 0,
	MIN_STATE_START = 1,
	MIN_STATE_DATA  = 2,
	MIN_STATE_END   = 3
} MIN_STATE;

typedef enum {
	MIN_PACKED_CMD    = 0,
	MIN_PACKED_HEADER = 1,
	MIN_PACKED_DATA   = 2
} PackedType;

typedef enum {
  MIN_CMD_START  = 0x10,  // goi mo dau
  MIN_CMD_END    = 0x13,  // goi header
  MIN_CMD_ACK    = 0x06,  // goi phan hoi hop le
  MIN_CMD_NACK   = 0x15   // goi phan hoi ko hop le
} CommandType;

typedef enum {
	MIN_LENGTH_CMD    = 0x01, //1 byte du lieu
	MIN_LENGTH_HEADER = 0x06, //6 byte du lieu (4 addr + 2 size)
	MIN_LENGTH_DATA   = 0x10, //16 byte du lieu
} DATA_LENGTH;

typedef struct {
  uint8_t  byte_start;   			// Start Byte (0xA5)
  PackedType type;   					// cmd type
  uint8_t length;   					// do dai du lieu
} __attribute__((packed)) PacketStartFormat;

typedef struct {
	uint8_t crc;         			// Checksum (2 byte)
  uint8_t  byte_stop;    			// Stop Byte (0x5A)
} __attribute__((packed)) PacketEndFormat;

typedef struct {
  PacketStartFormat start;  // 3 byte 0-1-2
  uint32_t address;     		// 4 byte 3-4-5-6
  uint16_t data_size;   		// 2 byte 7-8
	PacketEndFormat end;			// 2 byte 9-10
} __attribute__((packed)) HeaderPacket;

typedef struct {
  PacketStartFormat start;  // 3 byte
  uint8_t data[16];         // 16 byte
  PacketEndFormat end;      // 2 byte
} __attribute__((packed)) DataPacket;

typedef struct {
  PacketStartFormat start;  // 3 byte
	CommandType cmd;					// 1 byte
  PacketEndFormat end;      // 2 byte
} __attribute__((packed)) SimplePacket;

typedef struct {
	uint32_t no;
	uint32_t flash_start_addr;
	uint32_t flash_end_addr;
	uint32_t ram_addr;
}__attribute__((packed)) Information; 

void RCC_Init(void);
void ReadSimplePacket(SimplePacket *value);
void ReadHeaaderPacket(HeaderPacket *value);
void ReadDataPacket(DataPacket *value);
void Flash_Read(Information *value);
void Send_NACK(void);
void Send_ACK(void);
int main(void);

#endif
