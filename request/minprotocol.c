#include "minprotocol.h"

// Hàm tính CRC
uint8_t calculate_crc(uint8_t* data, size_t len) {
    uint8_t crc = 0;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
    }
    return crc;
}

// Hàm chuyển chuỗi hex thành số
uint32_t hexStringToUInt32(const char *hex) {
    uint32_t result = 0;
    for (int i = 0; i < 8; i++) {
        result <<= 4;
        if (hex[i] >= '0' && hex[i] <= '9') {
            result |= hex[i] - '0';
        } else if (hex[i] >= 'A' && hex[i] <= 'F') {
            result |= hex[i] - 'A' + 10;
        } else if (hex[i] >= 'a' && hex[i] <= 'f') {
            result |= hex[i] - 'a' + 10;
        }
    }
    return result;
}

uint16_t hexStringToUInt16(const char *hex) {
    uint16_t result = 0;
    for (int i = 0; i < 4; i++) {
        result <<= 4;
        if (hex[i] >= '0' && hex[i] <= '9') {
            result |= hex[i] - '0';
        } else if (hex[i] >= 'A' && hex[i] <= 'F') {
            result |= hex[i] - 'A' + 10;
        } else if (hex[i] >= 'a' && hex[i] <= 'f') {
            result |= hex[i] - 'a' + 10;
        }
    }
    return result;
}
// Hàm chuyển 2 ký tự hex thành 1 byte
unsigned char HexToByte(const char *hex) {
    unsigned char byte = 0;
    for (int i = 0; i < 2; i++) {
        byte <<= 4;
        if (hex[i] >= '0' && hex[i] <= '9') {
            byte |= hex[i] - '0';
        } else if (hex[i] >= 'A' && hex[i] <= 'F') {
            byte |= hex[i] - 'A' + 10;
        } else if (hex[i] >= 'a' && hex[i] <= 'f') {
            byte |= hex[i] - 'a' + 10;
        }
    }
    return byte;
}
// Hàm gửi SimplePacket
void send_simple_packet(HANDLE com_port, CommandType cmd) {
    SimplePacket packet = {
        .start = {START_BYTE, MIN_PACKED_CMD, MIN_LENGTH_CMD},
        .cmd = cmd,
        .end = {0, STOP_BYTE}
    };
    packet.end.crc = calculate_crc((uint8_t*)&packet, sizeof(SimplePacket) - sizeof(PacketEndFormat));
    SendData(com_port, (uint8_t*)&packet, sizeof(SimplePacket));
}

// Hàm gửi HeaderPacket
void send_header_packet(HANDLE com_port, uint32_t address, uint16_t data_size) {
    HeaderPacket packet = {
        .start = {START_BYTE, MIN_PACKED_HEADER, MIN_LENGTH_HEADER},
        .address = htonl(address), // Chuyển sang big-endian
        .data_size = htons(data_size), // Chuyển sang big-endian
        .end = {0, STOP_BYTE}
    };
    packet.end.crc = calculate_crc((uint8_t*)&packet, sizeof(HeaderPacket) - sizeof(PacketEndFormat));
    SendData(com_port, (uint8_t*)&packet, sizeof(HeaderPacket));
}

// Hàm gửi DataPacket
void send_data_packet(HANDLE com_port, uint8_t* data, size_t data_len) {
    DataPacket packet = {
        .start = {START_BYTE, MIN_PACKED_DATA, MIN_LENGTH_DATA},
        .end = {0, STOP_BYTE}
    };
    size_t copy_len = (data_len > 16) ? 16 : data_len;
    memcpy(packet.data, data, copy_len);
    if (copy_len < 16) {
        memset(packet.data + copy_len, 0xFF, 16 - copy_len); // Thay 0 bằng 0xFF theo yêu cầu trước
    }
    packet.end.crc = calculate_crc((uint8_t*)&packet, sizeof(DataPacket) - sizeof(PacketEndFormat));
    SendData(com_port, (uint8_t*)&packet, sizeof(DataPacket));
}
// Hàm xử lý state machine
void processCom(HANDLE com_port, MIN_STATE *host_state, const char *txtFilePath, int *currentLine) {
    switch (*host_state) {
        case MIN_STATE_IDLE: ;
            send_simple_packet(com_port, MIN_CMD_START);
            
            uint8_t response[sizeof(SimplePacket)];
            int bytesReceived = ReceiveData(com_port, response, sizeof(SimplePacket));
            
            if (bytesReceived > 0) {
                SimplePacket *respPacket = (SimplePacket*)response;
                if (respPacket->start.byte_start == START_BYTE && 
                    respPacket->end.byte_stop == STOP_BYTE &&
                    respPacket->start.type == MIN_PACKED_CMD) {
                    if (respPacket->cmd == MIN_CMD_ACK) {
                        printf("Nhan duoc ACK, chuyen sang MIN_STATE_START.\n");
                        *host_state = MIN_STATE_START;
                    } else if (respPacket->cmd == MIN_CMD_NACK) {
                        printf("Nhan duoc NACK, gui lai goi tin.\n");
                    }
                }
            } else {
                printf("Khong nhan duoc phan hoi, dong cong COM.\n");
                ClosePort(com_port);
                *host_state = MIN_STATE_IDLE;
            }
            break;

        case MIN_STATE_START: ;
            // Nhập 8 ký tự từ bàn phím làm địa chỉ
            char addressStr[9];
            printf("Nhap 8 ki tu hex lam dia chi (vi du: 20000000): ");
            fgets(addressStr, sizeof(addressStr), stdin);
            addressStr[strcspn(addressStr, "\r\n")] = '\0';
            if (strlen(addressStr) != 8) {
                printf("Phai nhap dung 8 ki tu!\n");
                ClosePort(com_port);
                *host_state = MIN_STATE_IDLE;
                break;
            }
            uint32_t address = hexStringToUInt32(addressStr);

            // Đọc file .st để lấy số byte dữ liệu
            FILE *txtFile = fopen(txtFilePath, "r");
            if (txtFile == NULL) {
                printf("Khong the mo file: %s\n", txtFilePath);
                ClosePort(com_port);
                *host_state = MIN_STATE_IDLE;
                break;
            }

            char line[256];
            if (fgets(line, sizeof(line), txtFile) == NULL) {
                printf("Khong the doc dong dau tien cua file: %s\n", txtFilePath);
                fclose(txtFile);
                ClosePort(com_port);
                *host_state = MIN_STATE_IDLE;
                break;
            }
            fclose(txtFile);

            line[strcspn(line, "\r\n")] = '\0';
            if (strlen(line) < 4) {
                printf("Dong dau tien phai co it nhat 4 ki tu: %s\n", line);
                ClosePort(com_port);
                *host_state = MIN_STATE_IDLE;
                break;
            }

            char sizeStr[5] = {0};
            strncpy(sizeStr, line, 4);
            uint16_t data_size = hexStringToUInt16(sizeStr);

            // Gửi HeaderPacket với địa chỉ nhập từ bàn phím
            send_header_packet(com_port, address, data_size);

            bytesReceived = ReceiveData(com_port, response, sizeof(SimplePacket));
            if (bytesReceived > 0) {
                SimplePacket *respPacket = (SimplePacket*)response;
                if (respPacket->start.byte_start == START_BYTE && 
                    respPacket->end.byte_stop == STOP_BYTE &&
                    respPacket->start.type == MIN_PACKED_CMD) {
                    if (respPacket->cmd == MIN_CMD_ACK) {
                        printf("Nhan duoc ACK, chuyen sang MIN_STATE_DATA.\n");
                        *host_state = MIN_STATE_DATA;
                        *currentLine = 1;
                    } else if (respPacket->cmd == MIN_CMD_NACK) {
                        printf("Nhan duoc NACK, gui lai goi tin.\n");
                    }
                }
            } else {
                printf("Khong nhan duoc phan hoi, dong cong COM.\n");
                ClosePort(com_port);
                *host_state = MIN_STATE_IDLE;
            }
            break;

        case MIN_STATE_DATA: ;
            FILE *txtFile2 = fopen(txtFilePath, "r");
            if (txtFile2 == NULL) {
                printf("Khong the mo file: %s\n", txtFilePath);
                ClosePort(com_port);
                *host_state = MIN_STATE_IDLE;
                break;
            }

            int lineNum = 0;
            while (fgets(line, sizeof(line), txtFile2)) {
                if (lineNum++ == *currentLine) break;
            }

            if (lineNum <= *currentLine) {
                printf("Het du lieu, chuyen sang MIN_STATE_END.\n");
                *host_state = MIN_STATE_END;
                fclose(txtFile2);
                break;
            }

            fclose(txtFile2);
            line[strcspn(line, "\r\n")] = '\0';

            uint8_t data[16];
            size_t len = strlen(line) / 2;
            if (len > 16) len = 16;
            for (size_t i = 0; i < len; i++) {
                data[i] = HexToByte(line + i * 2);
            }
            send_data_packet(com_port, data, len);

            bytesReceived = ReceiveData(com_port, response, sizeof(SimplePacket));
            if (bytesReceived > 0) {
                SimplePacket *respPacket = (SimplePacket*)response;
                if (respPacket->start.byte_start == START_BYTE && 
                    respPacket->end.byte_stop == STOP_BYTE &&
                    respPacket->start.type == MIN_PACKED_CMD) {
                    if (respPacket->cmd == MIN_CMD_ACK) {
                        printf("Nhan duoc ACK, gui dong tiep theo.\n");
                        (*currentLine)++;
                    } else if (respPacket->cmd == MIN_CMD_NACK) {
                        printf("Nhan duoc NACK, gui lai goi tin.\n");
                    }
                }
            } else {
                printf("Khong nhan duoc phan hoi, dong cong COM.\n");
                ClosePort(com_port);
                *host_state = MIN_STATE_IDLE;
            }
            break;

        case MIN_STATE_END: ;
            send_simple_packet(com_port, MIN_CMD_END);

            bytesReceived = ReceiveData(com_port, response, sizeof(SimplePacket));
            if (bytesReceived > 0) {
                SimplePacket *respPacket = (SimplePacket*)response;
                if (respPacket->start.byte_start == START_BYTE && 
                    respPacket->end.byte_stop == STOP_BYTE &&
                    respPacket->start.type == MIN_PACKED_CMD) {
                    if (respPacket->cmd == MIN_CMD_ACK) {
                        printf("Nhan duoc ACK, doi nhan HeaderPacket.\n");

                        uint8_t headerResponse[sizeof(HeaderPacket)];
                        int headerBytesReceived = ReceiveData(com_port, headerResponse, sizeof(HeaderPacket));
                        
                        if (headerBytesReceived == sizeof(HeaderPacket)) {
                            HeaderPacket *headerPacket = (HeaderPacket*)headerResponse;
                            uint8_t expectedCrc = calculate_crc(headerResponse, sizeof(HeaderPacket) - sizeof(PacketEndFormat));
                            
                            if (headerPacket->start.byte_start == START_BYTE &&
                                headerPacket->start.type == MIN_PACKED_HEADER &&
                                headerPacket->start.length == 0x06 &&
                                headerPacket->end.byte_stop == STOP_BYTE &&
                                headerPacket->end.crc == expectedCrc) {
                                printf("HeaderPacket dung cau truc, gui ACK va dong cong COM.\n");
                                send_simple_packet(com_port, MIN_CMD_ACK);
                                ClosePort(com_port);
                            } else {
                                printf("HeaderPacket khong dung cau truc, gui NACK va doi lai.\n");
                                send_simple_packet(com_port, MIN_CMD_NACK);
                            }
                        } else {
                            printf("Khong nhan duoc HeaderPacket day du, gui NACK.\n");
                            send_simple_packet(com_port, MIN_CMD_NACK);
                        }
                    } else if (respPacket->cmd == MIN_CMD_NACK) {
                        printf("Nhan duoc NACK, gui lai SimplePacket.\n");
                    }
                }
            } else {
                printf("Khong nhan duoc phan hoi, dong cong COM.\n");
                ClosePort(com_port);
                *host_state = MIN_STATE_IDLE;
            }
            break;
    }
}