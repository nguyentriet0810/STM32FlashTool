#include "RWfile.h"

// Hàm tìm kiếm file .hex trong thư mục và các thư mục con
int findHexFiles(const char *folderPath) {
    WIN32_FIND_DATA findFileData;
    char searchPath[MAX_PATH];
    int hexFileCount = 0; // Đếm số file .hex tìm thấy
    
    if (snprintf(searchPath, sizeof(searchPath), "%s\\*", folderPath) >= (int)sizeof(searchPath)) {
        printf("Duong dan qua dai: %s\n", folderPath);
        return 0;
    }
    
    HANDLE hFind = FindFirstFile(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        if (error != ERROR_FILE_NOT_FOUND) {
            printf("Khong the mo thu muc: %s (Error: %lu)\n", folderPath, error);
        }
        return 0;
    }

    do {
        if (strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..") == 0) {
            continue;
        }

        char fullPath[MAX_PATH];
        if (snprintf(fullPath, sizeof(fullPath), "%s\\%s", folderPath, findFileData.cFileName) >= (int)sizeof(fullPath)) {
            printf("Duong dan qua dai: %s\\%s\n", folderPath, findFileData.cFileName);
            continue;
        }

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            hexFileCount += findHexFiles(fullPath); // Cộng số file từ thư mục con
        } else if (strstr(findFileData.cFileName, ".hex") != NULL) {
            printf("Ten file: %s\nDuong dan: %s\n\n", findFileData.cFileName, fullPath);
            hexFileCount++;
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    return hexFileCount; // Trả về số file .hex tìm thấy
}

// Hàm tìm kiếm trong thư mục hiện tại
void findHexFilesInCurrentDir() {
    char currentDir[MAX_PATH];
    if (GetCurrentDirectory(sizeof(currentDir), currentDir) == 0) {
        printf("Khong the lay thu muc hien tai (Error: %lu)\n", GetLastError());
        return;
    }

    printf("Tim kiem file .hex trong thu muc: %s\n", currentDir);
    int hexFileCount = findHexFiles(currentDir);
    if (hexFileCount == 0) {
        printf("Khong co file .hex nao ton tai trong thu muc.\n");
    }
}

// Hàm tìm kiếm với đường dẫn nhập từ bàn phím
void findHexFilesFromInput() {
    char folderPath[MAX_PATH];
    printf("Nhap duong dan thu muc (vi du: C:\\Users\\Test): ");
    if (fgets(folderPath, sizeof(folderPath), stdin) == NULL) {
        printf("Loi khi nhap duong dan.\n");
        return;
    }

    size_t len = strlen(folderPath);
    if (len > 0 && folderPath[len - 1] == '\n') {
        folderPath[len - 1] = '\0';
    }

    DWORD attrib = GetFileAttributes(folderPath);
    if (attrib == INVALID_FILE_ATTRIBUTES || !(attrib & FILE_ATTRIBUTE_DIRECTORY)) {
        printf("Duong dan khong hop le hoac khong phai thu muc: %s\n", folderPath);
        return;
    }

    printf("Tim kiem file .hex trong thu muc: %s\n", folderPath);
    int hexFileCount = findHexFiles(folderPath);
    if (hexFileCount == 0) {
        printf("Khong co file .hex nao ton tai trong thu muc.\n");
    }
}

// Hàm chuyển 2 ký tự hex thành 1 byte
unsigned char hexToByte(const char *hex) {
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

// Hàm phân tích một dòng HEX thành HexFormat
int parseHexLine(const char *line, HexFileFormat *record) {
    if (line[0] != ':') {
        printf("Dong khong hop le: %s\n", line);
        return -1;
    }

    // Đọc độ dài (LL) - 2 ký tự từ vị trí 1
    record->length = hexToByte(line + 1);
    if (record->length > 16) { // Giới hạn tối đa của mảng data
        printf("Do dai du lieu qua lon: %d\n", record->length);
        return -1;
    }

    // Đọc địa chỉ (AAAA) - 4 ký tự từ vị trí 3
    record->address = (hexToByte(line + 3) << 8) | hexToByte(line + 5);

    // Đọc loại bản ghi (TT) - 2 ký tự từ vị trí 7
    record->recordType = hexToByte(line + 7);

    // Đọc dữ liệu (DD[]) - bắt đầu từ vị trí 9
    for (int i = 0; i < record->length; i++) {
        record->data[i] = hexToByte(line + 9 + i * 2);
    }

    // Đọc checksum (CC) - 2 ký tự cuối
    record->checksum = hexToByte(line + 9 + record->length * 2);

    // Kiểm tra checksum (tổng tất cả byte từ length đến data phải = 0 khi cộng với checksum)
    unsigned char sum = record->length + 
                       (record->address >> 8) + (record->address & 0xFF) + 
                       record->recordType;
    for (int i = 0; i < record->length; i++) {
        sum += record->data[i];
    }
    sum += record->checksum;
    if (sum != 0) {
        printf("Checksum khong hop le cho dong: %s\n", line);
        return -1;
    }

    return 0; // Thành công
}

// Hàm đọc file HEX, đếm số dòng, phân tích và tính tổng byte dữ liệu
void processHexFile(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        printf("Khong the mo file: %s\n", filePath);
        return;
    }

    char line[256];        // Buffer cho mỗi dòng (đủ lớn cho dòng HEX)
    int lineCount = 0;     // Đếm số dòng
    int totalBytes = 0;    // Tổng số byte dữ liệu (chỉ tính bản ghi loại 0x00)
    HexFileFormat record;  // Bản ghi tạm thời

    while (fgets(line, sizeof(line), file)) {
        // Xóa ký tự xuống dòng
        line[strcspn(line, "\r\n")] = '\0';

        // Bỏ qua dòng trống
        if (strlen(line) == 0) continue;

        // Phân tích dòng
        if (parseHexLine(line, &record) == 0) {
            lineCount++;
            // Chỉ tính byte dữ liệu cho bản ghi loại 0x00 (Data Record)
            if (record.recordType == 0x00) {
                totalBytes += record.length;
            }

            // In thông tin bản ghi (tùy chọn, có thể bỏ)
            printf("Line %02d: Length=%02X, Address=%04X, Type=%02X, Data=", 
                   lineCount, record.length, record.address, record.recordType);
            for (int i = 0; i < record.length; i++) {
                printf("%02X ", record.data[i]);
            }
            printf("Checksum=%02X\n", record.checksum);
        }
    }

    fclose(file);

    // In kết quả tổng hợp
    printf("\nTong so dong: %d\n", lineCount);
    printf("Tong so byte du lieu: %d\n", totalBytes);
}

// Hàm đọc file HEX và lưu dữ liệu vào file .txt
void saveHexDataToSt(const char *hexFilePath) {
    // Mở file HEX
    FILE *hexFile = fopen(hexFilePath, "r");
    if (hexFile == NULL) {
        printf("no such file: %s\n", hexFilePath);
        return;
    }

    // Tạo tên file .txt mới
    char stFilePath[260];
    strcpy(stFilePath, hexFilePath);
    char *dot = strrchr(stFilePath, '.');
    if (dot != NULL) {
        strcpy(dot, ".st");
    } else {
        strcat(stFilePath, ".st");
    }

    // Mở file .txt để ghi
    FILE *stFile = fopen(stFilePath, "w");
    if (stFile == NULL) {
        printf("Khong the tao file: %s\n", stFilePath);
        fclose(hexFile);
        return;
    }

        // Đọc file HEX và tính tổng byte dữ liệu
    char line[256];
    int lineCount = 0;
    int totalBytes = 0;
    HexFileFormat record;

    while (fgets(line, sizeof(line), hexFile)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;

        if (parseHexLine(line, &record) == 0) {
            lineCount++;
            if (record.recordType == 0x00) {
                totalBytes += record.length;
            }
        }
    }

    // Ghi dòng đầu tiên: số byte dữ liệu (hex)
    fprintf(stFile, "%04X\n", totalBytes);

    // Đặt lại con trỏ file HEX để đọc lại từ đầu
    rewind(hexFile);

    // Ghi dữ liệu từ các bản ghi loại 0x00
    while (fgets(line, sizeof(line), hexFile)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;

        if (parseHexLine(line, &record) == 0 && record.recordType == 0x00) {
            for (int i = 0; i < record.length; i++) {
                fprintf(stFile, "%02X", record.data[i]);
            }
            fprintf(stFile, "\n");
        }
    }

    fclose(hexFile);
    fclose(stFile);

    printf("Da luu du lieu vao file: %s\n", stFilePath);
    printf("Tong so dong: %d\n", lineCount);
    printf("Tong so byte du lieu: %d (0x%04X)\n", totalBytes, totalBytes);
}