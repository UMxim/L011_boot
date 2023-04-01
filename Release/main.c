#include "stm32l011xx.h"

#define _BOOT_VER 			0x01
#define _HW_TYPE 			0x02 // Тип устройства!!!!!!!! Важно! Заполнить для каждого устройства
#define _HW_VER  			0x03 // Железная версия устройства
#define _PAGE_SIZE_POW 		7 // Размер страницы 2^pow 7 == 128
#define _FW_PAGE_START 		0x08000200 // начало прошивки
#define _FLASH_SIZE 		0x4000 // полный размер флеша

#define UART 				LPUART1
//#define UART UART_USART2
#define UART_PORT 			GPIOA
#define UART_PIN 			UART_PIN_1
//#define UART_PIN UART_PIN_4
//#define UART_PIN UART_PIN_14
#define ANSWER_WAIT_MS 		512U // не более 8000 для этого МК
#define SYS_CLK_FREQ_KHZ 	2000U

// служебные
#define _PAGES_FOR_WRITE 	((0x08000000 + _FLASH_SIZE - _FW_PAGE_START) >> _PAGE_SIZE_POW)
#define ERASE 				(FLASH_PECR_ERASE | FLASH_PECR_PROG)

static void SendData(const uint8_t* data, uint32_t size)
{
	// enable Tx
	UART->CR1 = USART_CR1_TE | USART_CR1_UE;
	while (size--)
	{
		UART->TDR = *(data++);		
		while(!(UART->ISR & USART_ISR_TXE_Msk));
	}
	
	while(!(UART->ISR & USART_ISR_TC_Msk));
		
	//UART->CR1 = USART_CR1_UE;
}

static uint16_t ReceiveData(uint8_t *data)
{	
	UART->CR1 = USART_CR1_RE | USART_CR1_UE;
	while(!(UART->ISR & USART_ISR_RXNE_Msk));
	*data = UART->RDR;
	
	uint16_t size = (*data == 0xFF) ? (1<<_PAGE_SIZE_POW) + 2 : *data;
	data++;
	size += 3;	// sizeByte typeByte .. CheckSummByte
	
	for(int i=1; i<size; i++)
	{
		while(!(UART->ISR & USART_ISR_RXNE_Msk)); 
		*(data++) = UART->RDR;
	}	
	
	//UART->CR1 = USART_CR1_UE;	
	return size;
}

static void FlashWriteWord(uint32_t addr, uint32_t data, uint32_t isErase) // 0 или ERASE
{		
	FLASH->PECR = isErase;
	*(uint32_t *)addr = data;
	while ( (FLASH->SR & FLASH_SR_BSY) );
}


static void Go_To_User_App(void)
{
    uint32_t app_jump_address;
	
    typedef void(*pFunction)(void);//объявляем пользовательский тип
    pFunction Jump_To_Application;//и создаём переменную этого типа
		
		SCB->VTOR = _FW_PAGE_START;
	
    app_jump_address = *( uint32_t*) (_FW_PAGE_START + 4);    //извлекаем адрес перехода из вектора Reset
    Jump_To_Application = (pFunction)app_jump_address;            //приводим его к пользовательскому типу
    __set_MSP(*(__IO uint32_t*) _FW_PAGE_START);          //устанавливаем SP приложения                                           
    Jump_To_Application();		                        //запускаем приложение	
}

int main(void)
{	
	
// BR = 9600 !!!!!	
	// SysTick
	SysTick->LOAD = ANSWER_WAIT_MS * SYS_CLK_FREQ_KHZ;
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
	// UART
	RCC->APB1ENR = RCC_APB1ENR_LPUART1EN | RCC_APB1ENR_USART2EN; 
	RCC->IOPENR = RCC_IOPENR_GPIOAEN;
	UART->CR3 = USART_CR3_HDSEL;
	UART->BRR = 0xD055;
	// GPIO	
	UART_PORT->OTYPER = GPIO_OTYPER_OT_1; // OpenDrain
	UART_PORT->PUPDR |= GPIO_PUPDR_PUPD1_0;		
	UART_PORT->AFR[0] = 6 << GPIO_AFRL_AFSEL1_Pos;
	UART_PORT->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE1_Msk) | GPIO_MODER_MODE1_1; 
	
	//
	uint32_t buff32[(1<<_PAGE_SIZE_POW) + 5]; // потому-что памяти завались....
	uint8_t *buff = (uint8_t *)buff32;
	static const uint8_t firstPacket[] = {0x06, 0x47, _BOOT_VER, _HW_TYPE, _HW_VER, _PAGE_SIZE_POW, _PAGES_FOR_WRITE >> 8, _PAGES_FOR_WRITE,
										  0x06^ 0x47 ^_BOOT_VER ^_HW_TYPE ^_HW_VER ^_PAGE_SIZE_POW^(_PAGES_FOR_WRITE >> 8)^_PAGES_FOR_WRITE};
	SendData((void*)firstPacket, sizeof(firstPacket));
	
	UART->CR1 = USART_CR1_RE | USART_CR1_UE;
	
	while(!(UART->ISR & USART_ISR_RXNE_Msk))
	{
		if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
			Go_To_User_App();
	}
	
	FLASH->PEKEYR =  0x89ABCDEF;
	FLASH->PEKEYR =  0x02030405;
	FLASH->PRGKEYR = 0x8C9DAEBF;
	FLASH->PRGKEYR = 0x13141516;
	
	uint16_t size;
	uint8_t xor; 
	int i;
	static const uint8_t errCS[] =   {1, 0x10, 1, 0x10 ^ 1 };
	static const uint8_t errType[] = {1, 0x10, 2, 0x10 ^ 2 };
	while (1)
	{
		size = ReceiveData(buff);
		xor = 0;
		for (i=0; i<size; i++)
			xor ^= buff[i];
		if (xor)
		{					
			SendData(errCS, sizeof(errCS));
			continue;
		}
		
		if (buff[1] == 0xC7)
			NVIC_SystemReset();
	
		if (buff[1] != 0x88)
		{			
			SendData(errType, sizeof(errType));
			continue;
		}
	
		uint32_t addr = _FW_PAGE_START + (((buff[2]<<8) + buff[3]) << _PAGE_SIZE_POW);
		FlashWriteWord(addr, 0, ERASE);
		uint32_t word;
		for(i=0; i < (1<<_PAGE_SIZE_POW); i+=4)
		{			
			word = /*__REV*/(*(uint32_t*)&buff[4+i]);
			FlashWriteWord(addr, word, 0);
			addr += 4;
		}	
		buff[0] = 2;
		buff[1] = 0x08;
		buff[4] = buff[0] ^ buff[1] ^ buff[2] ^ buff[3];
		SendData(buff, 5);
	}	
}

