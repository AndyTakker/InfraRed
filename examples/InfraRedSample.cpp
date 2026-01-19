//==============================================================================
// Пример работы с IR-приемником на CH32V003.
// IR-приемник подключается к порту PD4.
//------------------------------------------------------------------------------
#include <InfraRed.h>
#include <debug.h>

InfraRed ir; // Объект, отвечающий за прием команд с IR-пульта

//==============================================================================
// Настройка прерывания
//------------------------------------------------------------------------------
void EXTI_INIT(void) {
  GPIO_InitTypeDef GPIO_InitStructure = {0};
  EXTI_InitTypeDef EXTI_InitStructure = {0};
  NVIC_InitTypeDef NVIC_InitStructure = {0};

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOD, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; // PD4
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  // GPIOD ----> EXTI_Line4
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource4);
  EXTI_InitStructure.EXTI_Line = EXTI_Line4;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI7_0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

#ifdef __cplusplus
extern "C" {
#endif
void EXTI7_0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI7_0_IRQHandler(void) {
  if (EXTI_GetITStatus(EXTI_Line4) != RESET) { // На PD4 сигнал изменился из 1 в 0
    ir.receive();                              // Обработчик приема пакета
    EXTI_ClearITPendingBit(EXTI_Line4);
  }
}
#ifdef __cplusplus
}
#endif

//==============================================================================
int main() {
  // Стандартные приседания
  SystemCoreClockUpdate();
  Delay_Init();
  Delay_Ms(4000); // Ждем, пока раздуплится терминал в VSCode

  USART_Printf_Init(115200);
  printf("SystemClk: %lu\r\n", SystemCoreClock);      // Для посмотреть частоту процесора (48мГц)
  printf("   ChipID: %08lx\r\n", DBGMCU_GetCHIPID()); // Для посмотреть ID чипа, от нефиг делать

  EXTI_INIT(); // Настройка прерываний

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
