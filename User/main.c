#include "debug.h"
#include "dfplayer.h"
#include "display.h"
#include "fonts.h"

/* Global define */
#define FOLDER_MIN  1
#define FOLDER_MAX  99
#define TRACK_MIN   1
#define TRACK_MAX   999
#define VOLUME_MIN  0
#define VOLUME_MAX  30

/* Global Variable */
extern uint8_t rxBuffer[DFPLAYER_UART_FRAME_SIZE];
extern uint8_t rxPos;
extern uint8_t dfpReady;
extern uint8_t dfpDone;
extern uint8_t dfpOk;
extern uint8_t dfpSource;
extern uint16_t dfpError;

extern const uint8_t font8x8[][8];

uint8_t folder = 2;
uint16_t track = 1;
uint16_t volume = 15;

/**
 * @brief Setup USART1
 */
void initUSART1() {
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1, ENABLE);

  // USART1 TX --> D.5
  GPIO_InitTypeDef initUsartTx = {0};
  initUsartTx.GPIO_Pin = GPIO_Pin_5;
  initUsartTx.GPIO_Speed = GPIO_Speed_30MHz;
  initUsartTx.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOD, &initUsartTx);

  // USART1 RX --> D.6 */
  GPIO_InitTypeDef initUsartRx = {0};
  initUsartRx.GPIO_Pin = GPIO_Pin_6;
  initUsartRx.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOD, &initUsartRx);

  // USART1
  USART_InitTypeDef initUsart = {0};
  initUsart.USART_BaudRate = 9600;
  initUsart.USART_WordLength = USART_WordLength_8b;
  initUsart.USART_StopBits = USART_StopBits_1;
  initUsart.USART_Parity = USART_Parity_No;
  initUsart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  initUsart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_Init(USART1, &initUsart);

  // NVIC
  NVIC_InitTypeDef initNvic = {0};
  initNvic.NVIC_IRQChannel = USART1_IRQn;
  initNvic.NVIC_IRQChannelPreemptionPriority = 0;
  initNvic.NVIC_IRQChannelSubPriority = 0;
  initNvic.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&initNvic);

  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  USART_Cmd(USART1, ENABLE);
}

/**
 * @brief Init I2C1
 */
void initI2C1() {
  // Pins
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  
  GPIO_InitTypeDef initI2C = {0};
  initI2C.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
  initI2C.GPIO_Mode = GPIO_Mode_AF_OD;
  initI2C.GPIO_Speed = GPIO_Speed_30MHz;
  GPIO_Init(GPIOC, &initI2C);

  GPIO_InitTypeDef initButtons = {0};
  initButtons.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
  initButtons.GPIO_Mode = GPIO_Mode_IPU;
  initButtons.GPIO_Speed = GPIO_Speed_30MHz;
  GPIO_Init(GPIOC, &initButtons);

  // I2C
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

  I2C_InitTypeDef initTypeDef = {0};
  initTypeDef.I2C_ClockSpeed = DISPLAY_I2C_SPEED;
  initTypeDef.I2C_Mode = I2C_Mode_I2C;
  initTypeDef.I2C_DutyCycle = I2C_DutyCycle_2;
  initTypeDef.I2C_OwnAddress1 = 0x00;
  initTypeDef.I2C_Ack = I2C_Ack_Enable;
  initTypeDef.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_Init(I2C1, &initTypeDef);

  I2C_Cmd(I2C1, ENABLE);
}

/**
 * @fn      USART1_IRQHandler
 * @brief   This function handles USART1 global interrupt request.
 */
void USART1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART1_IRQHandler(void) {
  if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
    rxBuffer[rxPos] = USART_ReceiveData(USART1);
    if (rxPos == 0 && rxBuffer[rxPos] != DFPLAYER_UART_START_BYTE) {
      return;
    }
    rxPos++;

    if (rxPos == DFPLAYER_UART_FRAME_SIZE) {
      dfplayer_return();
      rxPos = 0;
    }
  }
}

/**
 * @brief Display show information
 */
void displayShow() {
  char buff[17];
  uint8_t line[128];

  sprintf(buff, "Folder: %2d     ", folder);
  text(buff, line);
  displaySendData(0, line, sizeof(line));

  sprintf(buff, "Track: %3d      ", track);
  text(buff, line);
  displaySendData(1, line, sizeof(line));

  sprintf(buff, "S: %1d E: %6d ", dfpSource, dfpError);
  text(buff, line);
  displaySendData(2, line, sizeof(line));

  sprintf(buff, "R: %1d D: %1d O: %1d ", dfpReady, dfpDone, dfpOk);
  text(buff, line);
  displaySendData(3, line, sizeof(line));
}

/**
 * @brief Main function
 *
 */
int main(void) {
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  SystemCoreClockUpdate();
  Delay_Init();
#if (SDI_PRINT == SDI_PR_OPEN)
  SDI_Printf_Enable();
#else
  USART_Printf_Init(115200);
#endif
  printf("SystemClk: %d\r\n", SystemCoreClock);
  printf("ChipID: %08x\r\n", DBGMCU_GetCHIPID());

  initUSART1();
  initI2C1();
  
  displayInit();
  dfplayer_reset();
  //Delay_Ms(DFPLAYER_BOOT_DELAY);
  while (dfpReady == 0) {
    Delay_Ms(100);
  }
  
  printf("Set volume\r\n");
  dfplayer_setVolume(volume);
  Delay_Ms(DFPLAYER_CMD_DELAY);

  dfplayer_repeatFolder(folder);
  
  //dfplayer_repeatAll(1);

  while (1) {
    displayShow();

    Delay_Ms(200);
  }
}
