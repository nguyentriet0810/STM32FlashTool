#ifndef __MINPROTOCOL_H__
#define __MINPROTOCOL_H__

#include "com.h"

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
	CommandType cmd;	    // 1 byte
  PacketEndFormat end;      // 2 byte
} __attribute__((packed)) SimplePacket;

uint8_t calculate_crc(uint8_t* data, size_t len);
uint32_t hexStringToUInt32(const char *hex);
uint16_t hexStringToUInt16(const char *hex);
unsigned char HexToByte(const char *hex);
void send_simple_packet(HANDLE com_port, CommandType cmd);
void send_header_packet(HANDLE com_port, uint32_t address, uint16_t data_size);
void send_data_packet(HANDLE com_port, uint8_t* data, size_t data_len);
void processCom(HANDLE com_port, MIN_STATE *host_state, const char *txtFilePath, int *currentLine);

#endif /*__MINPROTOCOL_H__*/