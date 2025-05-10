#ifndef __COM_H__
#define __COM_H__

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <winsock2.h>
#include <tchar.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <conio.h>
#include <ctype.h>

HANDLE OpenPort(int idx, int baud_rate);
void ClosePort(HANDLE com_port);
int SendData(HANDLE com_port, const uint8_t *data, size_t length);
int ReceiveData(HANDLE com_port, uint8_t *buffer, size_t len);
void listAvailablePorts(void);

#endif /*__COM_H__*/
