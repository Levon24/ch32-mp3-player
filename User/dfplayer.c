#include <stdio.h>
#include <ch32v00x.h>
#include <string.h>
#include "debug.h"
#include "dfplayer.h"

const enum dfplayer_module module = DFPLAYER_MINI; 
const uint8_t ack = 0x01; // 0x01 = module return feedback after the command, 0x00 = module not return feedback after the command

uint8_t txBuffer[DFPLAYER_UART_FRAME_SIZE];
uint8_t rxBuffer[DFPLAYER_UART_FRAME_SIZE];
uint8_t rxPos = 0;

/*****************************************************************************
 * @brief Write buffer to USART1
 * 
 *****************************************************************************/
void dfplayer_write(uint8_t size) {
	for (uint8_t p = 0; p < size; p++) {
		USART_SendData(USART1, txBuffer[p]);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {
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
	txBuffer[0] = DFPLAYER_UART_START_BYTE;
  txBuffer[1] = DFPLAYER_UART_VERSION;
  txBuffer[2] = DFPLAYER_UART_DATA_LEN;
  txBuffer[3] = cmd;
  txBuffer[4] = ack;
  txBuffer[5] = dh;
  txBuffer[6] = dl;

	int16_t checksum;
	switch (module) {
    case DFPLAYER_MINI:
    case DFPLAYER_HW_247A:
      checksum = 0x0000; // 0x0000, DON'T TOUCH!!!
      checksum = checksum - txBuffer[1] - txBuffer[2] - txBuffer[3] - txBuffer[4] - txBuffer[5] - txBuffer[6];
      break;

    case DFPLAYER_FN_X10P:
      checksum = 0xFFFF; // 0xFFFF, DON'T TOUCH!!!
      checksum = checksum - txBuffer[1] - txBuffer[2] - txBuffer[3] - txBuffer[4] - txBuffer[5] - txBuffer[6] + 1;
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
      txBuffer[7] = checksum >> 8;
      txBuffer[8] = checksum;
      txBuffer[9] = DFPLAYER_UART_END_BYTE;
			dfplayer_write(DFPLAYER_UART_FRAME_SIZE);

			// GD3200B/MH2024K chip so slow & need delay after write command
      if (module == DFPLAYER_HW_247A) {	
				Delay_Ms(DFPLAYER_CMD_DELAY);
			}
      break;

    case DFPLAYER_NO_CHECKSUM:
    default:
      txBuffer[7] = DFPLAYER_UART_END_BYTE;
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
	memset(rxBuffer, 0, DFPLAYER_UART_FRAME_SIZE);
	
	for (uint8_t p = 0; p < size; p++) {
		while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET) {
			/* waiting for receiving finish */
		}
		txBuffer[p] = USART_ReceiveData(USART1);
	}
}

/*****************************************************************************
 * @brief Get response from player
 * 
 *****************************************************************************/
uint16_t dfplayer_response(uint8_t cmd) {
	dfplayer_read(DFPLAYER_UART_FRAME_SIZE);
	
	if (rxBuffer[3] == cmd) {
		// return DH, DL
		return ((uint16_t) rxBuffer[5] << 8) | rxBuffer[6];
	} else {
		return 0;
	}
}

/*****************************************************************************
 * @brief Play next track in chronological order
 * NOTE:
 *  - files in the root must start with 4 decimal digits with leading zeros
 *  - example: SD_ROOT/0001 - My favorite song.mp3
 *  - don’t copy 0003.mp3 & then 0001.mp3, because 0003.mp3 will be played first
 *****************************************************************************/
void dfplayer_playNext() {
	dfplayer_send(DFPLAYER_PLAY_NEXT, 0, 0);
}

/*****************************************************************************
 * @brief Play previous track in chronological order
 * NOTE:
 *  - files in the root must start with 4 decimal digits with leading zeros
 *  - example: SD_ROOT/0001 - My favorite song.mp3
 *  - don’t copy 0003.mp3 & then 0001.mp3, because 0003.mp3 will be played first
 *****************************************************************************/
void dfplayer_playPrevious() {
	dfplayer_send(DFPLAYER_PLAY_PREVIOUS, 0, 0);
}

/*****************************************************************************
 * @brief Play tracks by number, sorted in chronological order (1-3000/9999)
 * NOTE:
 *  - files in the root must start with 4 decimal digits with leading zeros
 *  - example: SD_ROOT/0001 - My favorite song.mp3
 *  - don’t copy 0003.mp3 & then 0001.mp3, because 0003.mp3 will be played first
 *****************************************************************************/
void dfplayer_playTrack(uint16_t track) {
	dfplayer_send(DFPLAYER_PLAY_TRACK, (track >> 8), track);
}

/*****************************************************************************
 * @brief Pause current track
 *****************************************************************************/
void dfplayer_pause() {
	dfplayer_send(DFPLAYER_PAUSE, 0, 0);
}

/*****************************************************************************
 * @brief Increase volume
 *****************************************************************************/
void dfplayer_volumeUp() {
	dfplayer_send(DFPLAYER_SET_VOLUME_UP, 0, 0);
}

/*****************************************************************************
 * @brief Decrease volume
 *****************************************************************************/
void dfplayer_volumeDown() {
	dfplayer_send(DFPLAYER_SET_VOLUME_DOWN, 0, 0);
}

/*****************************************************************************
 * @brief Specify volume
 * NOTE: Volume level: 0-30
 *****************************************************************************/
void dfplayer_setVolume(uint8_t volume) {
	dfplayer_send(DFPLAYER_SET_VOLUME, 0, volume);
}
