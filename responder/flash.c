#include <flash.h>

void Flash_WriteWord(uint32_t address, uint32_t data) {
    FLASH_Unlock();
    FLASH_ProgramWord(address, data);
    FLASH_Lock();
}

void Flash_Erase(uint32_t addresspage) {
    FLASH_Unlock();
    while (FLASH_GetFlagStatus(FLASH_FLAG_BSY) == SET);
    FLASH_Status status = FLASH_ErasePage(addresspage);
    if (status != FLASH_COMPLETE) { // ko xoa dc thi xu ly loi
        FLASH_Lock();
        return; 
    }
    FLASH_Lock();
}
void Flash_WriteNumByte(uint32_t address, uint8_t *data, int num) {
    FLASH_Unlock();  
    while (FLASH_GetFlagStatus(FLASH_FLAG_BSY) == SET);  //cho flash san sang
    for (int i = 0; i < num / 2; i++) { // neu so byte can ghi la so chan
        uint16_t halfword = data[2 * i] | (data[2 * i + 1] << 8);
        FLASH_ProgramHalfWord(address + 2 * i, halfword);
        while (FLASH_GetFlagStatus(FLASH_FLAG_BSY) == SET); // Ch? ghi xong
    }
    if (num % 2) { //neu so byte can ghi la so le
        uint16_t lastHalfWord = data[num - 1] | 0xFF00; // Ghep byte cuoi voi 0xFF
        FLASH_ProgramHalfWord(address + num - 1, lastHalfWord);
        while (FLASH_GetFlagStatus(FLASH_FLAG_BSY) == SET);
    }
    FLASH_Lock();  // Khóa Flash sau khi ghi
}
