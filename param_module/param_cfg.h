#ifndef PARAM_CFG_H_
#define PARAM_CFG_H_
/*
Модуль для хранения настроек и статистики, а также изменение параметров по команде с UART
Для работы необходимо:

2. Задать PARAM_FLASH_OFFSET - начало сохраненных параметров во флеш или EEPROM 
3. Задать PARAM_TIMER_PERIOD_MKS. Необходимо по таймеру вызывать Param_Timer_callback. Этим дефайном задается перииод вызова.
4. Заполнить массивы params и statistic по образцу в param_cfg.c 
4а.Чтобы не использовать обезличенные переменные вроде param[0].value можно задать им клички типа #define STAT_UPDATE_FREQ_S
5. Написать интерфейсные функции Param_SendByte Param_GetCRC16 Param_WriteParam Param_Reset для работы с текущим железом
6. Описать ф-ию Param_GetDump(), которая вызывается при команде получения дампа
7. В обработчике прерываний получения/отправки байта по UART вызывать ф-ии Param_RecieveByte_callback Param_TransmitByte_callback
8. В обработчике прерываний таймера вызвать Param_Timer_callback

Порядок работы модуля
1. Проверка сохраненных данных на FLASH. При корректности - заполнение переменных param и statistic
2. С периодичностью STAT_UPDATE_FREQ_S проверяется изменение статистики. Если да - то сохранение на FLASH
3. При поступлении команды по UART выполнить действия и отдать ответ

*/

#define PARAM_FLASH_OFFSET 0x12345678 // адрес нулевого смещения
#define PARAM_TIMER_PERIOD_MKS 1000// периодичность вызова таймера Param_Timer_callback() в мкс
#define PARAM_IS_STATIC_BUFF 1 // 1- сразу выделяем массив на 260, 0- будем использовать malloc 
// заполнить псевдонимы для переменных параметров. по образцу:
#define STAT_UPDATE_FREQ_S param[0].value	// это нужный параметр - частота проверки изменения статистики и запись на флеш при необходимости

#define STAT_ON_CNT statistic[0].value



params_t params[]; // параметры
params_t statistic[]; // статистика

// описать следующие функции, зависящие от HW 

void Param_SendByte(uint8_t byte);

#if (IS_HW_CRC)
uint16_t Param_GetCRC16(const void *data, uint16_t size);
#endif

int Param_WriteParam(uint32_t offset, const void *data, uint16_t size);

void Param_Reset(void);

uint32_t Param_GetDump(uint8_t* dump);

#endif // PARAM_CFG_H_