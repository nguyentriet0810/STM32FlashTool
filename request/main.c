#include "main.h"

int main(int argc, char *argv[]) {
    if(argc < 2) {
        print_hala();
    }
    else if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        print_version();
    }
    else if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        print_help();
    }
    else if (argc == 2 && strcmp(argv[1], "--listcom") == 0) {
        listAvailablePorts();
    }
    else if (argc == 2 && strcmp(argv[1], "--fhex") == 0) {
        findHexFilesInCurrentDir();
    }
    else if (argc == 2 && strcmp(argv[1], "--findhex") == 0) {
        findHexFilesFromInput();
    }
    else if (argc == 2 && strcmp(argv[1], "--convert") == 0) {
        char hexFilePath[260];
        printf("Nhap duong dan day du cua file .hex can chuyen doi (vi du: C:/Users/Test) ");
        fgets(hexFilePath, sizeof(hexFilePath), stdin);
        hexFilePath[strcspn(hexFilePath, "\r\n")] = '\0';
        saveHexDataToSt(hexFilePath);
    }
    else if (argc == 2 && strcmp(argv[1], "--opencom") == 0) {
        char *buffer = (char *)malloc(16 * sizeof(char));
        int *idx = (int *)malloc(1 * sizeof(int));
        int *baud_rate = (int *)malloc(1 * sizeof(int));
        
        if (buffer == NULL || idx == NULL || baud_rate == NULL) {
            printf("Memory allocation failed!\n");
            return 1;
        }
        printf("Please enter your select com port: (VD: 9,112500)");
        fgets(buffer, 16, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';
        // Tách số nguyên và số thực từ chuỗi nhập vào
        if (sscanf(buffer, "%d,%d", idx, baud_rate) == 2) {
            OpenPort(*idx,*baud_rate);
        }
        else {
            printf("ERROR: Format eror. Please check and try it again");
        }
        free(buffer);
        free(idx);
        free(baud_rate);
    }
    else if (argc == 2 && strcmp(argv[1], "--readhex") == 0) {
        char hexFilePath[260];
        printf("Please enter your path   ");
        fgets(hexFilePath, sizeof(hexFilePath), stdin);
        hexFilePath[strcspn(hexFilePath, "\r\n")] = '\0';
        processHexFile(hexFilePath);
    }
    else if (argc == 2 && strcmp(argv[1], "--upload") == 0) {

    }
    else {
        printf("ERROR: No command line\n");
        printf("'--help' for more information\n");
    }
    return 0;
}
void print_hala(void) {
    printf("hala.exe: fatal error: no input files\n");
    printf("compilation terminated.\n");
}
void print_version(void) {
    printf("hala.exe 1.0.0\n");
    printf("Copyright (C305) 2025 Free Software\n");
}
void print_help(void) {
    printf("Usage: hala.exe [options] file...\n");
    printf("Options:\n");
    printf("  --help                    Display this information.\n");
    printf("  --version                 Display compiler version information.\n");
    printf("  --fhex                    Find '.hex' file in current folder.\n");
    printf("  --findhex                 Find '.hex' file in file path.\n");
    printf("  --convert                 Convert '.hex' file to '.st' file.\n");
    printf("  --listcom                 List com port.\n");
    printf("  --upload                  Upload firmware form .st file to stm32.\n");
    printf("  --opencom                 Open com port.\n");
    printf("  --readhex                 Read '.hex' file.\n");
    printf("\nFor more information, please see:\n");
    printf("Zalo: ZeroEightFiveTwoSevenThreeZeroSevenNineNine. \n");
    printf("Or <https://www.youtube.com/@hoangtriet9999>.\n");
}

