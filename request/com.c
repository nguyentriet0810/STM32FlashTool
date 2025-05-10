#include "com.h"

// Hàm mở cổng COM
HANDLE OpenPort(int idx, int baud_rate) {
    HANDLE hComm;
    TCHAR comname[100];
    wsprintf(comname, TEXT("\\\\.\\COM%d"), idx);
    hComm = CreateFile(comname, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hComm == INVALID_HANDLE_VALUE) {
        printf("Khong the mo cong COM%d\n", idx);
        return INVALID_HANDLE_VALUE;
    }
    DCB dcb = {0};
    GetCommState(hComm, &dcb);
    dcb.BaudRate = baud_rate;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    SetCommState(hComm, &dcb);

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 100;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    SetCommTimeouts(hComm, &timeouts);

    printf("Da mo cong COM%d thanh cong.\n", idx);
    return hComm;
}

// Hàm đóng cổng COM
void ClosePort(HANDLE com_port) {
    if (com_port != INVALID_HANDLE_VALUE) {
        CloseHandle(com_port);
        printf("Da dong cong COM.\n");
    }
}

// Hàm gửi dữ liệu nhị phân
int SendData(HANDLE com_port, const uint8_t *data, size_t length) {
    DWORD dNoOfBytesWritten;
    if (!WriteFile(com_port, data, length, &dNoOfBytesWritten, NULL)) {
        printf("Loi khi gui du lieu: %lu\n", GetLastError());
        return -1;
    }
    printf("Da gui %u byte: ", (unsigned int)dNoOfBytesWritten);
    for (size_t i = 0; i < dNoOfBytesWritten; i++) {
        printf("%02X ", data[i]); // In dữ liệu dưới dạng hex
    }
    printf("\n");
    return (int)dNoOfBytesWritten;
}

int ReceiveData(HANDLE com_port, uint8_t *buffer, size_t len) {
    DWORD bytesRead;
    if (!ReadFile(com_port, buffer, len, &bytesRead, NULL)) {
        printf("Loi khi nhan du lieu: %lu\n", GetLastError());
        return -1;
    }
    if (bytesRead == 0) return 0;
    printf("Nhan duoc %u byte: ", (unsigned int)bytesRead);
    for (size_t i = 0; i < bytesRead; i++) {
        printf("%02X ", buffer[i]);
    }
    printf("\n");
    return (int)bytesRead;
}

// Hàm liệt kê các cổng COM có sẵn
void listAvailablePorts(void)
{
    printf("Available COM Ports:\n");
    for (int i = 1; i <= 255; i++)
    {
        char comPortName[10];
        sprintf(comPortName, "COM%d", i);

        HANDLE hPort = CreateFileA(
            comPortName,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        if (hPort != INVALID_HANDLE_VALUE)
        {
            printf("%s is available\n", comPortName);
            CloseHandle(hPort);
        }
    }
}
