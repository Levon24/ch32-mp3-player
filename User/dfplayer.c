#include <stdio.h>
#include <ch32v00x.h>
#include <string.h>
#include "debug.h"
#include "dfplayer.h"

uint8_t buffer[DFPLAYER_UART_FRAME_SIZE];
uint8_t ack = 0x01; // 0x01 = module return feedback after the command, 0x00 = module not return feedback after the command
enum dfplayer_module module = DFPLAYER_MINI; 

/*****************************************************************************
 * @brief Write buffer to USART1
 * 
 *****************************************************************************/
void dfplayer_write(uint8_t size) {
	for (uint8_t p = 0; p < size; p++) {
		USART_SendData(USART1, buffer[p]);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {
			/* waiting for sending finish */
		}
	}
}

/*****************************************************************************
 * @brief Send data to player
 * Send data via Serial port
 *   NOTE:
 *   - DFPlayer TX data frame format:
 *     0      1    2    3    4    5   6   7     8     9-byte
 *     START, VER, LEN, CMD, ACK, DH, DL, SUMH, SUML, END
 *            -------- checksum --------
 *
 *****************************************************************************/
void dfplayer_send(uint8_t cmd, uint8_t dh, uint8_t dl) {
	buffer[0] = DFPLAYER_UART_START_BYTE;
  buffer[1] = DFPLAYER_UART_VERSION;
  buffer[2] = DFPLAYER_UART_DATA_LEN;
  buffer[3] = cmd;
  buffer[4] = ack;
  buffer[5] = dh;
  buffer[6] = dl;

	int16_t checksum;
	switch (module) {
    case DFPLAYER_MINI:
    case DFPLAYER_HW_247A:
      checksum = 0x0000; // 0x0000, DON'T TOUCH!!!
      checksum = checksum - buffer[1] - buffer[2] - buffer[3] - buffer[4] - buffer[5] - buffer[6];
      break;

    case DFPLAYER_FN_X10P:
      checksum = 0xFFFF; // 0xFFFF, DON'T TOUCH!!!
      checksum = checksum - buffer[1] - buffer[2] - buffer[3] - buffer[4] - buffer[5] - buffer[6] + 1;
      break;

    case DFPLAYER_NO_CHECKSUM:
    default:
      //empty - no checksum calculation, not recomended for MCU without external crystal oscillator
      break;
  }

  switch (module) {
    case DFPLAYER_MINI:
    case DFPLAYER_FN_X10P:
    case DFPLAYER_HW_247A:
      buffer[7] = checksum >> 8;
      buffer[8] = checksum;
      buffer[9] = DFPLAYER_UART_END_BYTE;
			dfplayer_write(DFPLAYER_UART_FRAME_SIZE);

			// GD3200B/MH2024K chip so slow & need delay after write command
      if (module == DFPLAYER_HW_247A) {	
				Delay_Ms(DFPLAYER_CMD_DELAY);
			}
      break;

    case DFPLAYER_NO_CHECKSUM:
    default:
      buffer[7] = DFPLAYER_UART_END_BYTE;
      dfplayer_write(DFPLAYER_UART_FRAME_SIZE - 2); // -2 =SUMH & SUML not used
      break;
  }
}

/*****************************************************************************
 * @brief Read data from player
 * Read MP3 player command feedback
 * NOTE:
 *  - command feedback timeout 100msec(YX5200/AAxxxx)..350msec(GD3200B/MH2024K)
 *  - DFPlayer RX data frame format:
 *     0      1    2    3    4    5   6   7     8     9-byte
 *     START, VER, LEN, CMD, ACK, DH, DL, SUMH, SUML, END
 * 
 *****************************************************************************/
void dfplayer_read(uint8_t size) {
	memset(buffer, 0, DFPLAYER_UART_FRAME_SIZE);
	
	for (uint8_t p = 0; p < size; p++) {
		while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET) {
			/* waiting for receiving finish */
		}
		buffer[p] = USART_ReceiveData(USART1);
	}
}

/*****************************************************************************
 * @brief Get response from player
 * 
 *****************************************************************************/
uint16_t dfplayer_response(uint8_t cmd) {
	dfplayer_read(DFPLAYER_UART_FRAME_SIZE);
	
	if (buffer[3] == cmd) {
		// return DH, DL
		return ((uint16_t) buffer[5] << 8) | buffer[6];
	} else {
		return 0;
	}
}

/*****************************************************************************
 * @brief Play next files
 * 
 *****************************************************************************/
void dfplayer_next() {
	dfplayer_send(DFPLAYER_PLAY_NEXT, 0, 0);
}
