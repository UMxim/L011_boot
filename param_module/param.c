#include "param.h"
#include "param_cfg.h"

#define PARAM_LIST_NUM ( sizeof(params) / sizeof(params_t) )
#define STAT_LIST_NUM ( sizeof(statistic) / sizeof(params_t) )
#define STAT_OFFSET (PARAM_FLASH_OFFSET +  sizeof(params_t) + 4) // 4 для выравнивания!!!!
#define BUFF_MAX_SIZE (1 + 1 + 255 + 2 ) // T L V CRC16
#define STATE_NO_INIT	0
#define STATE_IDLE 		1
#define STATE_RX   		2
#define STATE_TX		3	
#define US_IN_S		1000000UL
#define SAFE_TIMER_INTERVAL 300000UL // 300ms

typedef const* char str_t;
static const str_t* errors[] = {
	"", // 0
	"", // 1
};

typedef struct {
	uint32_t value;
	const char * name;
} params_t;

#if (PARAM_IS_STATIC_BUFF)
static uint8_t buff[BUFF_MAX_SIZE];
#else 
static uint8_t *buff;
#endif
static uint32_t timer_us = 0;
static uint32_t timer_s = 0;
static uint8_t state = STATE_NO_INIT;
static uint32_t safeTimer = 0;
static uint16_t counter = 0; // позиция приема/отправки

uint16_t Param_GetCRC16(const void *data, uint16_t size)
{
	
	
}

static void Fill_Arr_(uint32_t flashOffset, uint16_t N_elem, params_t *dest)
{
	uint32_t *data = (uint32_t *)flashOffset;		
	uint32_t size = N_elem * sizeof(uint32_t) + 2;
	
	if ( Param_GetCRC16(data, size) != 0 )	{
		return;
	}
	
	// read data
	for (int i=0; i< N_elem; i++) {
		dest->value = *data;
		data++;
	}		
}

// Возврат буфера
static void GetBuff_(uint32_t size)
{
#if (PARAM_IS_STATIC_BUFF == 0 
if (size > BUFF_MAX_SIZE) {
	return NULL;
}
buff = malloc(size);
#endif
return;
}

static void FreeBuff(void *buff)
{
#if (PARAM_IS_STATIC_BUFF == 0)
	free(buff);
#endif
}

static void Update_Stat_(void)
{
	
}
static void SetError(uint8_t errN)
{
	
}

static void ParseBuff_(void)	
{
	// check crc
	if (Param_GetCRC16(buff, buff[1]) != 0 )	{
		
	}
}

static void SendBuff_(void)
{
	state_ = STATE_TX;
}

void Param_Init(void)
{
	// read params	
	Fill_Arr_(PARAM_FLASH_OFFSET, PARAM_LIST_NUM, &params);	
	// read statistic
	Fill_Arr_(STAT_OFFSET, STAT_LIST_NUM, &statistic);	
	state_ = STATE_IDLE;	
}

// Вызываем в прерывании при получении байта
void Param_RecieveByte_callback(byte)
{	
	static uint8_t firstByte;	
	if (state == STATE_NO_INIT) return;
	if ( (state == STATE_IDLE) || (state == STATE_RX) ) ; else return;	
	
	switch (counter)	{
		case 0:			
			firstByte = byte;
			counter++;
			state = STATE_RX;
			safeTimer = timer_us;
			break;
		case 1:			
			GetBuff_(1 + 1 + byte + 2);
			if (!buff) {
				state = STATE_NO_INIT; // вот и всё
				return;
			}
			buff[0] = firstByte;
			buff[1] = byte;
			counter++;
			break;		
		default:
			buff[counter]= byte;
			counter++;			
			if (counter == (2 + buff[1] + 2)) {	// все получено, обрабатываем				
				ParseBuff_();	
				SendBuff_();		
			}
			break;
	}	
}

// Вызывать в прерывании по отправке байта
void Param_TransmitByte_callback(void)
{
	if (state == STATE_NO_INIT) return;
	
}

// Вызывать в прерывании таймера. Квант времени описать в дефайне 
void Param_Timer_callback(void)
{	
	static uint32_t nextStatUpdate = STAT_UPDATE_FREQ_S;
	if (state == STATE_NO_INIT) return;
	timer_us += PARAM_TIMER_PERIOD_MKS;
	uint32_t fullSec = US_IN_S * timer_s;
	if ( (timer_us - fullSec) >= US_IN_S ) {
		timer_s++;
	}
	if (timer_s >= nextStatUpdate) {
		nextStatUpdate+=STAT_UPDATE_FREQ_S;
		Update_Stat_();
	}
	if ( (timer_us - safeTimer) > SAFE_TIMER_INTERVAL )...................
}