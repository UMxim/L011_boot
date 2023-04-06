/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32l011xx.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef hlpuart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_LPUART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define _BOOT_VER 			0x01
#define _HW_TYPE 			0x02 // Тип устройства!!!!!!!! Важно! Заполнить для каждого устройства
#define _HW_VER  			0x03 // Железная версия устройства
#define _PAGE_SIZE_POW 		7 // Размер страницы 2^pow 7 == 128
#define _FW_PAGE_START 		0x08000400 // начало прошивки
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

static void SendData(uint8_t* data, uint32_t size)
{
	// enable Tx
	UART->CR1 = USART_CR1_TE | USART_CR1_UE;
	while (size--)
	{
		UART->TDR = *(data++);		
		while(!(UART->ISR & USART_ISR_TXE_Msk));
	}
	
	while(!(UART->ISR & USART_ISR_TC_Msk));
		
	UART->CR1 = USART_CR1_UE;
}

static uint16_t ReceiveData(uint8_t *data)
{	
	UART->CR1 = USART_CR1_RE | USART_CR1_UE;
	while(!(UART->ISR & USART_ISR_RXNE_Msk));
	*data = UART->RDR;
	
	uint16_t size = (*data == 0xFF) ? (1<<_PAGE_SIZE_POW) + 2 : *data;
	size += 3;	// sizeByte typeByte .. CheckSummByte
	data++;
	for(int i=1; i<size; i++)
	{
		while(!(UART->ISR & USART_ISR_RXNE_Msk)); 
		*(data++) = UART->RDR;
	}	
	
	UART->CR1 = USART_CR1_UE;	
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

#define UPDATE_MASK(reg, mask, val) reg = ( (reg) & ~(mask) ) | (val)
	#define UART 				LPUART1
	#define UART_PORT 			GPIOA	
	#define GPIO_OPEN_DRAIN		GPIO_OTYPER_OT_1
	#define GPIO_PULL_UP_MSK	GPIO_PUPDR_PUPD1_Msk
	#define GPIO_PULL_UP_VAL	GPIO_PUPDR_PUPD1_0
	#define GPIO_AFR_NUM		0	// 0..7 pin
	#define GPIO_AFR_MSK		GPIO_AFRL_AFSEL1_Msk
	#define GPIO_AFR			6 // AF6 для LPUART1
	#define GPIO_AFR_POS		GPIO_AFRL_AFSEL1_Pos
	#define GPIO_MODER_MSK		GPIO_MODER_MODE1_Msk
	#define GPIO_MODER_VAL      GPIO_MODER_MODE1_1
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	// SysTick
	SysTick->LOAD = 0x123456;//ANSWER_WAIT_MS * SYS_CLK_FREQ_KHZ;
	SysTick->VAL = 0;//ANSWER_WAIT_MS * SYS_CLK_FREQ_KHZ;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
	
	// UART
	RCC->APB1ENR = RCC_APB1ENR_LPUART1EN | RCC_APB1ENR_USART2EN; 
	RCC->IOPENR = RCC_IOPENR_GPIOAEN;
	UART->CR3 = USART_CR3_HDSEL;
	UART->BRR = 0xD055;
	// GPIO	
	UART_PORT->OTYPER |= GPIO_OPEN_DRAIN; // OpenDrain 1
	UPDATE_MASK(UART_PORT->PUPDR, GPIO_PULL_UP_MSK, GPIO_PULL_UP_VAL);		//2
	UPDATE_MASK(UART_PORT->AFR[GPIO_AFR_NUM], GPIO_AFR_MSK, GPIO_AFR << GPIO_AFR_POS);
	UPDATE_MASK(UART_PORT->MODER, GPIO_MODER_MSK, GPIO_MODER_VAL);
	
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
	
	FLASH->PEKEYR = 0x89ABCDEF;
	FLASH->PEKEYR = 0x02030405;
	FLASH->PRGKEYR = 0x8C9DAEBF;
	FLASH->PRGKEYR = 0x13141516;
	
	uint16_t size;
	uint8_t xor; 
	int i;
	while (1)
	{
		size = ReceiveData(buff);
		xor = 0;
		for (i=0; i<size; i++)
			xor ^= buff[i];
		if (xor)
		{
			buff[0] |= (1<<4);
			buff[1] = 1;
			buff[2] = 1; // Err ChSumm
			buff[3] = buff[0] ^ 1 ^ 1;
			SendData(buff, 4);
			continue;
		}
		
		if (buff[1] == 0xC7)
			NVIC_SystemReset();
	
		if (buff[1] != 0x88)
			continue;
	
		uint32_t addr = _FW_PAGE_START + (((buff[2]<<8) + buff[3]) << _PAGE_SIZE_POW);
		FlashWriteWord(addr, 0, ERASE);
		uint32_t word;
		for(i=0; i < (1<<_PAGE_SIZE_POW); i+=4)
		{			
			word = (*(uint32_t*)&buff[4+i]);
			FlashWriteWord(addr, word, 0);
			addr += 4;
		}		
		buff[0] = 0x02;
		buff[1] = 0x08;
		buff[4] = buff[0] ^ buff[1] ^ buff[2] ^ buff[3];
		SendData(buff, 5);
	}	
	
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_LPUART1_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief LPUART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 9600;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_HalfDuplex_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
