#include <main.h>

#define RX_BUFFER_SIZE 64
uint8_t rx_buffer[RX_BUFFER_SIZE];
RINGBUFF_TypeDef ring_buffer;

volatile uint8_t uart3_idle_detected = 0;
uint8_t test;
uint32_t current_byte = 0;

int main(void) {
	RCC_Init();
	UART3_IDLE_Init();
	RINGBUFF_Init(&ring_buffer, rx_buffer, RX_BUFFER_SIZE);
	uint8_t device_state = MIN_STATE_IDLE;
	Information inf;
	
	while(1) {
		switch (device_state) 
		{
			case MIN_STATE_IDLE:
				if (uart3_idle_detected) {
					uart3_idle_detected = 0;
          SimplePacket command;
					ReadSimplePacket(&command);
					if(command.start.byte_start != START_BYTE || command.start.type != MIN_PACKED_CMD 
							|| command.start.length != MIN_LENGTH_CMD || command.cmd != MIN_CMD_START 
							|| command.end.byte_stop != STOP_BYTE) {
						Send_NACK();
					} else if (RINGBUFF_Get(&ring_buffer,&test) != RING_FAIL) {
						while(RINGBUFF_Get(&ring_buffer,&test) == RING_OK) {};
						Send_NACK();
					} else {
						device_state = MIN_STATE_START;
						Send_ACK();
					}
        }
				break;
			case MIN_STATE_START:
				if (uart3_idle_detected) {
					uart3_idle_detected = 0;
					static uint8_t page;
					HeaderPacket header;
					ReadHeaaderPacket(&header);
					if(header.start.byte_start != START_BYTE || header.start.type != MIN_PACKED_HEADER
						|| header.start.length != MIN_LENGTH_HEADER || header.end.byte_stop != STOP_BYTE) {
						Send_NACK();
					} else if (RINGBUFF_Get(&ring_buffer,&test) != RING_FAIL) {
						while(RINGBUFF_Get(&ring_buffer,&test) == RING_OK) {};
						Send_NACK();
					}else {
						device_state = MIN_STATE_DATA;
						page = header.data_size/0x400;
						if ((header.data_size%0x400) != 0) {
							page++;
						}
						Flash_Read(&inf);
						if(inf.no != 0xFFFFFFFF){
							//chuan bi page trong de ghi du lieu
							for(int i = 0; i < page; i++) {
								Flash_Erase(inf.flash_end_addr + i*0x400);
							}
							Flash_WriteWord((app_infor_start_address + (inf.no+1)*16),inf.no + 1);
							Flash_WriteWord((app_infor_start_address + (inf.no+1)*16)+4,inf.flash_end_addr);
							Flash_WriteWord((app_infor_start_address + (inf.no+1)*16)+8,inf.flash_end_addr + page*0x400);
							Flash_WriteWord((app_infor_start_address + (inf.no+1)*16)+12,header.address);
						} else {
							for(int i = 0; i < page; i++) {
								Flash_Erase(app_start_address + i*0x400);
							}
							Flash_WriteWord(app_infor_start_address    , 0);
							Flash_WriteWord(app_infor_start_address + 4, app_start_address);
							Flash_WriteWord(app_infor_start_address + 8, app_start_address + page*0x400);
							Flash_WriteWord(app_infor_start_address +12, header.address);
						}
						Send_ACK();
					}
				}
				break;
			case MIN_STATE_DATA:
				if (uart3_idle_detected) {
					uart3_idle_detected = 0;
					uint8_t buffer[21];
					for(int i = 0; i < 21; i++) {
						RINGBUFF_Get(&ring_buffer,&test);
						buffer[i] = test;
					}
					if (RINGBUFF_Get(&ring_buffer,&test) != RING_FAIL) {
						while(RINGBUFF_Get(&ring_buffer,&test) == RING_OK) {};
						Send_NACK();
					} else if (buffer[0] != START_BYTE) {
						Send_NACK();
					} else {
						if (buffer[1] == MIN_PACKED_CMD) {
							if (buffer[2] == MIN_LENGTH_CMD && buffer[3] == MIN_CMD_END && buffer[5] == STOP_BYTE) {
								device_state = MIN_STATE_END;
								Send_ACK();
							} else {
								Send_NACK();
							}
						} else if (buffer[1] == MIN_PACKED_DATA) {
							if (buffer[2] == MIN_LENGTH_DATA && buffer[20] == STOP_BYTE) {
								Flash_Read(&inf);
								Flash_WriteNumByte((inf.flash_start_addr + current_byte*MIN_LENGTH_DATA),&buffer[3],MIN_LENGTH_DATA);
								current_byte++;
								Send_ACK();
							} else {
								Send_NACK();
							}
						} else {
							Send_NACK();
						}
					}
				}
				break;
			case MIN_STATE_END:
				device_state = MIN_STATE_IDLE;
				break;
			default:
				break;
		}
	}
}
void USART3_IRQHandler(void) {
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
		uint8_t byte = USART_ReceiveData(USART3);
    RINGBUFF_Put(&ring_buffer, byte);
    USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	}
	if (USART_GetITStatus(USART3, USART_IT_IDLE) != RESET) {
		uart3_idle_detected = 1;
    USART_ReceiveData(USART3);  // doc dl xoa idle
    USART_ClearITPendingBit(USART3, USART_IT_IDLE);
  }
}
void ReadSimplePacket(SimplePacket *value) {
	static uint8_t byte = 0;
	if(RINGBUFF_Get(&ring_buffer,&byte) == RING_OK) {
		value->start.byte_start = byte;
	} else return;
	if(RINGBUFF_Get(&ring_buffer,&byte) == RING_OK) {
		value->start.type = (PackedType)byte;
	} else return;
	if(RINGBUFF_Get(&ring_buffer,&byte) == RING_OK) {
		value->start.length = byte;
	} else return;
	if(RINGBUFF_Get(&ring_buffer,&byte) == RING_OK) {
		value->cmd = (CommandType)byte;
	} else return;
	if(RINGBUFF_Get(&ring_buffer,&byte) == RING_OK) {
		value->end.crc = byte;
	} else return;
	if(RINGBUFF_Get(&ring_buffer,&byte) == RING_OK) {
		value->end.byte_stop = byte;
	} else return;
}
void ReadHeaaderPacket(HeaderPacket *value) {
	static uint8_t byte = 0;
	if(RINGBUFF_Get(&ring_buffer,&byte) == RING_OK) {
		value->start.byte_start = byte;
	} else return;
	if(RINGBUFF_Get(&ring_buffer,&byte) == RING_OK) {
		value->start.type = (PackedType)byte;
	} else return;
	if(RINGBUFF_Get(&ring_buffer,&byte) == RING_OK) {
		value->start.length = byte;
	} else return;
	for(int i = 0; i < 4; i++) {
		if(RINGBUFF_Get(&ring_buffer,&byte) == RING_OK) {
				value->address = (value->address<<8)|byte;
		} else return;
	}
	for(int i = 0; i < 2; i++) {
		if(RINGBUFF_Get(&ring_buffer,&byte) == RING_OK) {
			value->data_size = (value->data_size<<8)|byte;
		} else return;
	}
	if(RINGBUFF_Get(&ring_buffer,&byte) == RING_OK) {
		value->end.crc = byte;
	} else return;
	if(RINGBUFF_Get(&ring_buffer,&byte) == RING_OK) {
		value->end.byte_stop = byte;
	} else return;
}
void ReadDataPacket(DataPacket *value) {
	static uint8_t byte = 0;
	if(RINGBUFF_Get(&ring_buffer,&byte) == RING_OK) {
		value->start.byte_start = byte;
	} else return;
	if(RINGBUFF_Get(&ring_buffer,&byte) == RING_OK) {
		value->start.type = (PackedType)byte;
	} else return;
	if(RINGBUFF_Get(&ring_buffer,&byte) == RING_OK) {
		value->start.length = byte;
	} else return;
	for(int i = 0; i < value->start.length; i++) {
		if(RINGBUFF_Get(&ring_buffer,&byte) == RING_OK) {
				value->data[i] = byte;
		} else return;
	}
	if(RINGBUFF_Get(&ring_buffer,&byte) == RING_OK) {
		value->end.crc = byte;
	} else return;
	if(RINGBUFF_Get(&ring_buffer,&byte) == RING_OK) {
		value->end.byte_stop = byte;
	} else return;
}
void Flash_Read(Information *value) {
	uint32_t inf = app_infor_start_address;
	Information data;
	uint8_t flag = 0;
	while(inf <= (uint32_t)app_infor_end_address) {
		uint32_t check = *(volatile uint32_t*)inf;
		if(check != 0xFFFFFFFF && check < 64) {
			data.no = *(volatile uint32_t*)inf;
			data.flash_start_addr = *(volatile uint32_t*)(inf + 4);
			data.flash_end_addr = *(volatile uint32_t*)(inf + 8);
			data.ram_addr = *(volatile uint32_t*)(inf + 12);
			flag = 1;
		}
		inf += 16;
	}
	if(flag) {
		value->no               = data.no;
		value->flash_start_addr = data.flash_start_addr;
		value->flash_end_addr   = data.flash_end_addr;
		value->ram_addr         = data.ram_addr;
	} else {
		value->no               = 0xFFFFFFFF;
		value->flash_start_addr = 0xFFFFFFFF;
		value->flash_end_addr   = 0xFFFFFFFF;
		value->ram_addr         = 0xFFFFFFFF;
	}
}
void Send_NACK(void) {
  SimplePacket nack_packet;
  nack_packet.start.byte_start = START_BYTE;
  nack_packet.start.type = MIN_PACKED_CMD;
  nack_packet.start.length = 1;
  nack_packet.cmd = MIN_CMD_NACK;
  nack_packet.end.crc = 0;  
  nack_packet.end.byte_stop = STOP_BYTE;
  UART3_SendBuffer((uint8_t *)&nack_packet, sizeof(nack_packet));
}
void Send_ACK(void) {
	SimplePacket ack_packet;
	ack_packet.start.byte_start = START_BYTE;
	ack_packet.start.type = MIN_PACKED_CMD;
  ack_packet.start.length = 1;
  ack_packet.cmd = MIN_CMD_ACK;
  ack_packet.end.crc = 0;  
  ack_packet.end.byte_stop = STOP_BYTE;
  UART3_SendBuffer((uint8_t *)&ack_packet, sizeof(ack_packet));
}
void RCC_Init(void) {
	//HSE on
	RCC->CR |= RCC_CR_HSEON;
	while(!(RCC->CR & RCC_CR_HSERDY));
	//PLLMUX x9 aks 8Mx9=72M
	RCC->CFGR |= RCC_CFGR_PLLMULL9;
	//input clock pll
	RCC->CFGR |= RCC_CFGR_PLLSRC_HSE;
	//APB1, ABP2
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;
	//PLLon
	RCC->CR |= RCC_CR_PLLON;
	while(!(RCC->CR & RCC_CR_PLLRDY));
	//input systick is pll
	RCC->CFGR |= RCC_CFGR_SWS_PLL;
	while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
	//waait 2T
	FLASH->ACR |= FLASH_ACR_LATENCY;
	//buffer
	FLASH->ACR |= FLASH_ACR_PRFTBE;
	while(!(FLASH->ACR & FLASH_ACR_PRFTBS));
}


