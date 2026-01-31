//==============================================================================
// Пример работы с IR-приемником на CH32V003.
// IR-приемник подключается к порту PD4.
// Принцип использования библиотеки:
// 1. Создать объект InfraRed ir
// 2. В обработчике прерываний EXTI7_0_IRQHandler вызвать ir.receive()
//    Вызывать всегда, ir.recieve() сам проверит, его это прерывание или чужое.
// 3. В цикле в main проверять ir.ready() и если пакет принят, то
//    прочитать нужные параметры и проверить при необходимости.
//------------------------------------------------------------------------------
#include <InfraRed.h>
#include <debug.h>

InfraRed ir(PD4); // Объект, отвечающий за прием команд с IR-пульта на пине PD4

#ifdef __cplusplus
extern "C" {
#endif
void EXTI7_0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI7_0_IRQHandler(void) {
  ir.receive(); // Обработчик приема пакета с IR-приемника
}
#ifdef __cplusplus
}
#endif

//==============================================================================
int main() {
  // Стандартные приседания
  SystemCoreClockUpdate();
  Delay_Init();
  USART_Printf_Init(115200);
  printf("SystemClk: %lu\r\n", SystemCoreClock);      // Для посмотреть частоту процесора (48мГц)
  printf("   ChipID: %08lx\r\n", DBGMCU_GetCHIPID()); // Для посмотреть ID чипа, от нефиг делать

  while (1) {
    // Ждем приема пакета из IR-порта
    if (ir.ready()) { // если пакет принят
      printf("Packet: %08lX, Command: %02X, Address: %02X \r\n", ir.readPacket(), ir.readCommand(), ir.readAddress());
      // Если нужно, сюда можно добавить проверку принятых данных с помощью функций:
      // ir.chkParity(); ir.chkAddress(); ir.chkCommand();
      // Что считать правильным - решать индивидуально с учетом конкретного пульта.
    }
  }
}
