#ifndef __RWfile_H__
#define __RWfile_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <winsock2.h>

// Định nghĩa cấu trúc HexFormat
typedef struct 
{
    unsigned char length;       // Độ dài (LL)
    unsigned short address;     // Địa chỉ (AAAA)
    unsigned char recordType;   // Loại bản ghi (TT)
    unsigned char data[16];     // Dữ liệu (DD[])
    unsigned char checksum;     // Checksum (CC)
} HexFileFormat;

int findHexFiles(const char *folderPath);
void findHexFilesInCurrentDir();
void findHexFilesFromInput();
unsigned char hexToByte(const char *hex);
int parseHexLine(const char *line, HexFileFormat *record);
void processHexFile(const char *filePath);
void saveHexDataToSt(const char *hexFilePath);

#endif /*__RWfile_H__*/