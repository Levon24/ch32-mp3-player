#include "debug.h"
#include "dfplayer.h"

/* Global define */


/* Global Variable */

/*****************************************************************************
 * @brief Setup USART1
 *
 *****************************************************************************/
void initUSART1() {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1, ENABLE);

	// USART1 TX --> D.5
	GPIO_InitTypeDef initUsartTx = {0};
	initUsartTx.GPIO_Pin = GPIO_Pin_5;
	initUsartTx.GPIO_Speed = GPIO_Speed_30MHz;
	initUsartTx.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOD, &initUsartTx);

	// USART1 RX --> D.6 */
	GPIO_InitTypeDef initUartRx = {0};
	initUartRx.GPIO_Pin = GPIO_Pin_6;
	initUartRx.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &initUartRx);

	USART_InitTypeDef initUsart = {0};
	initUsart.USART_BaudRate = 9600;
	initUsart.USART_WordLength = USART_WordLength_8b;
	initUsart.USART_StopBits = USART_StopBits_1;
	initUsart.USART_Parity = USART_Parity_No;
	initUsart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	initUsart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART1, &initUsart);

	USART_Cmd(USART1, ENABLE);
}

/*****************************************************************************
 * @brief Main function
 *
 *****************************************************************************/
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

	Delay_Ms(3000);
	dfplayer_next();

	Delay_Ms(500);

	while (1) {
	}
}
