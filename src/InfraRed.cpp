//==============================================================================
// Структура NEC пакета: 32 бита, 4 блока по 8 бит, LSB<-MSB
// [START] [адрес-8бит] [~адрес-8бит] [команда-8бит] [~команда-8бит][END-опционально]
// Но не все пульты соблюдают это правило, хотя по параметрам вроде NEC.
//------------------------------------------------------------------------------
#include "InfraRed.h"

InfraRed::InfraRed(PinName pin) {
  _pin = pin;
  timerInit();
  pinMode(_pin, GPIO_Mode_IPU);
  pinExtiInit(_pin, EXTI_Trigger_Falling);
}

void InfraRed::timerInit() {
  // Настройка TIM2 на 1 МГц (1 мкс тик)
  RCC->APB1PCENR |= RCC_APB1Periph_TIM2;       // Включить тактирование TIM2
  TIM2->PSC = (SystemCoreClock / 1000000) - 1; // 47 при 48 МГц
  TIM2->ATRLR = 0xFFFF;                        // Автозагрузка (максимум)
  TIM2->SWEVGR = 1;                            // reset prescaler
  TIM2->CNT = 0;                               // Сброс счётчика
  TIM2->CTLR1 = TIM_CounterMode_Up;            // Режим up-counting
  TIM2->CTLR1 |= TIM_CEN;
}

void InfraRed::receive(void) {
  if (EXTI_GetITStatus(extiLine(_pin)) == RESET) { // На "нашем" пине сигнал не изменился
    return; // Не наше прерывание, уходим.
  }
  EXTI_ClearITPendingBit(extiLine(_pin)); // Сброс прерывания

  uint16_t time = TIM2->CNT; // Интервал с предыдущего прерывания
  TIM2->CNT = 0;             // И обнулим счетчик

  if (time > _NEC_START_MIN && time < _NEC_START_MAX) { // Длительность стартового импульса.
    _start = true;                                      // Начинаем прием нового пакета
    _buffer = 0;                                        // Почистим буфер
    _counter = 0;                                       // Сбросим счетчик принятых бит
    return;
  }
  // Если мы сюда добрались, значит получен один из содержательных импульсов пакета
  uint8_t bit = (time >= _NEC_HIGH_MIN && time <= _NEC_HIGH_MAX) ? 1 : 0; // Проверка на HIGH или LOW импульс
  _buffer = _buffer << 1 | bit;                                           // Поместим принятый бит в буфер
  if (++_counter == _NEC_PACKET_SIZE) {                                   // Если приняли все биты пакета
    _start = false;                                                       // завершаем чтение
    _packet = _buffer;                                                    // Копируем буфер в пакет
    _buffer = 0;                                                          // И почистим буфер
    _ready = true;                                                        // Признак наличия принятого пакета
    return;
  }
}

//==============================================================================
// Признак наличия принятого пакета. Возвращает true, если принят пакет.
// Признак сбрасывается после вызова данной функции.
//------------------------------------------------------------------------------
bool InfraRed::ready() {
  if (_ready) {
    _ready = false;
    return true;
  }
  return false;
}

uint32_t InfraRed::readPacket() {
  return _packet;
}

uint8_t InfraRed::readAddress() {
  return ((uint32_t)_packet >> 24);
}

uint8_t InfraRed::readCommand() {
  return ((uint32_t)_packet >> 8 & 0xFF);
}

//==============================================================================
// Проверка четности единиц в пакете.
// Возвращает true, если число единиц в пакете нечетное, и false, если четное.
//------------------------------------------------------------------------------
// bool InfraRed::chkParity(uint32_t x) {
bool InfraRed::chkParity() {
  uint32_t x = _packet;
  x ^= x >> 16;
  x ^= x >> 8;
  x ^= x >> 4;
  x ^= x >> 2;
  x ^= x >> 1;
  return x & 1;
}

//==============================================================================
// Проверка адреса. Возвращает true, если адрес прямой соответствует адресу
// инверсному.
//------------------------------------------------------------------------------
// bool InfraRed::chkAddress(uint32_t x) {
bool InfraRed::chkAddress() {
  uint8_t b0 = (_packet >> 24) & 0xFF; // старший байт (адрес)
  uint8_t b1 = (_packet >> 16) & 0xFF; // ~адрес
  return (b0 ^ b1) == 0xFF;
}

//==============================================================================
// Проверка команды. Возвращает true, если команда прямая соответствует команде
// инверсной.
//------------------------------------------------------------------------------
// bool InfraRed::chkCommand(uint32_t x) {
bool InfraRed::chkCommand() {
  uint8_t b2 = (_packet >> 8) & 0xFF; // старший байт (команда)
  uint8_t b3 = (_packet) & 0xFF;      // ~команда
  return (b2 ^ b3) == 0xFF;
}
