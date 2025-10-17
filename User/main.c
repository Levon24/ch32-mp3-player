#include "debug.h"
#include "dfplayer.h"

/* Global define */


/* Global Variable */
extern uint8_t rxBuffer[DFPLAYER_UART_FRAME_SIZE];
extern uint8_t rxPos;
extern uint8_t dfpReady;

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

	USART_InitTypeDef initUsart = {0};
	initUsart.USART_BaudRate = 9600;
	initUsart.USART_WordLength = USART_WordLength_8b;
	initUsart.USART_StopBits = USART_StopBits_1;
	initUsart.USART_Parity = USART_Parity_No;
	initUsart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	initUsart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART1, &initUsart);

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

	dfplayer_reset();
	//Delay_Ms(DFPLAYER_BOOT_DELAY);
	while (dfpReady == 0) {
		Delay_Ms(100);
	}
	
	printf("Set volume\r\n");
	dfplayer_setVolume(12);
	Delay_Ms(DFPLAYER_CMD_DELAY);

	dfplayer_repeatFolder(2);
	
	//dfplayer_repeatAll(1);

	while (1) {
		printf("Ping\r\n");
		//dfplayer_playNext();

		Delay_Ms(30000);
	}
}
