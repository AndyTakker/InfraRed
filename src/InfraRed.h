//==============================================================================
// Библиотека для работы с ИК протоколом NEC.
// Идея почерпнута у Гайвера: https://github.com/GyverLibs/NecDecoder
// Библиотека реализована для работы на CH32V003 без Arduino-framework.
// Поскольку куча пультов поддерживает протокол NEC не полностью, в
// библиотеке реализован подход: сигнал принимается как есть, а проверки
// корректности пакета выполняются на прикладном уровне в зависимости от
// конкретного используемого пульта.
// Для этого в библиотеке есть необходимые функции: проверка четности,
// корректность адреса, корректность команды.
// Временные параметры сигнала используем NEC.
//
// Особенности:
// - функция receive() должна вызываться в прерывании от входного сигнала
//   c IR-приемника.
// - для расчета длительности импульсов необходимо в конструктор передать функцию,
//   которая считает микросекунды. Например: Sysclock.Micros() или свою.
// - режим повтора не реализован (пока ни разу не потребовался).
//------------------------------------------------------------------------------
#pragma once

#include <ch32Pins.hpp>
#include <ch32v00x.h>

#define _NEC_PACKET_SIZE 32  // размер пакета (бит)
#define _NEC_TOLERANCE 150   // допуск high/low, мкс
#define _NEC_TOLERANCE2 1500 // допуск start/repeat, мкс

// Тайминги NEC, мкс
#define _NEC_HIGH_BIT 2250
#define _NEC_LOW_BIT 1150
#define _NEC_START_BIT 14400

// =========================================================================
#define _NEC_HIGH_MIN (_NEC_HIGH_BIT - _NEC_TOLERANCE)
#define _NEC_HIGH_MAX (_NEC_HIGH_BIT + _NEC_TOLERANCE)
#define _NEC_LOW_MIN (_NEC_LOW_BIT - _NEC_TOLERANCE)
#define _NEC_LOW_MAX (_NEC_LOW_BIT + _NEC_TOLERANCE)
#define _NEC_START_MIN (_NEC_START_BIT - _NEC_TOLERANCE2)
#define _NEC_START_MAX (_NEC_START_BIT + _NEC_TOLERANCE2)

class InfraRed {
  public:
  InfraRed(PinName pin, uint32_t (*getMicros)());
  void receive();        // Вызывать при переходе из High в Low (Falling) на пине ИК приемника в прерывании
  bool ready();          // Возвращает true, если пакет прочитан
  uint32_t readPacket(); // Прочитать пакет целиком (адрес + ~адрес + команда + ~команда)
  uint8_t readAddress(); // Прочитать адрес
  uint8_t readCommand(); // Прочитать команду
  bool chkParity();      // Проверка четности пакета
  bool chkAddress();     // Проверка адреса
  bool chkCommand();     // Проверка команды

  private:
  PinName _pin;                                // Имя пина ИК приемника (в стиле PD4, PD5 и т.д.)
  uint32_t _lastTime = 0;                      // Момент предыущего фронта импульса
  volatile uint32_t _packet = 0;               // Буфер последнего принятого пакета (4 байта)
  volatile uint32_t _buffer = 0;               // Буфер текущего принимаемого пакета (4 байта)
  volatile int8_t _counter = _NEC_PACKET_SIZE; // Счетчик бит в принимаемом пакете
  volatile bool _ready = false;                // Флаг готовности данных к чтению
  volatile bool _start = false;                // старт флаг
  uint32_t (*m_getMicros)();                   // Функция получения времени в микросекундах 
};
