#ifndef PARAM_H_
#define PARAM_H_

// Инициализация. Вызвать после инициализации железа но до других модулей (Заполняет параметры сохраненными значениями!!!)
void Param_Init(void);

// А её нет! Все происходит по прерываниям! Даже запись во флеш (все равно затупим)
//void Param_Cycle(void);

// Вызываем в прерывании при получении байта
void Param_RecieveByte_callback(byte);

// Вызвать в прерывании при завершении отправки байта
void Param_TransmitByte_callback(void);

// Вызывать в прерывании таймера. Квант времени описать в дефайне 
void Param_Timer_callback(void);

#endif // PARAM_H_