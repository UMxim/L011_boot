#include "param_cfg.h"

#define PARAM_MEMBER(def_val, name) {def_val; name; }

params_t params[] = {
	// заполнить по аналогии. не забыть в .h записать allias этой переменной
	PARAM_MEMBER( 24*60*60, "Stat_upd_time, sec"); // STAT_UPDATE_FREQ_S по умолчанию - сутки 
	
}

params_t statistic[] = {
	// заполнить по аналогии. не забыть в .h записать allias этой переменной
	//PARAM_MEMBER( 0, "on counter"); // количество включений
	
}

// HW описать функции взаимодействия с железом
// Отослать байт в UART
void Param_SendByte(uint8_t byte)
{
	
}

// пользовательская ф-ия вычисления crc. иначе сработает weak
#if (IS_HW_CRC)
uint16_t Param_GetCRC16(const void *data, uint16_t size)
{
	/// тут своя функция обсчета crc . 
}
#endif

// работа с flash/ eeprom
int Param_WriteParam(uint32_t offset, const void *data, uint16_t size)
{
	offset += PARAM_FLASH_OFFSET;
	// записать данные со смещением. смещение 0 == PARAM_FLASH_OFFSET
}

void Param_Reset(void)
{
	// _NVIC_Reset(); или подобное
}

// Вызывается при команде получения дампа.
// [param out] dump - адрес для копирования дампа
// [return] Размер дампа в байтах. 0- дампа нет
uint32_t Param_GetDump(uint8_t* dump)
{
	return 0;
}
/// Не забыть в прерывании вызвать :
/// По приему байта :   Param_RecieveByte_callback(byte);
/// По отправке байта : Param_TransmitByte_callback();