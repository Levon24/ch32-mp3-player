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

/*****************************************************************************
 * @brief Specify equalizer (0/1/2/3/4/5)
 * NOTE: 0:Normal/1:Pop/2:Rock/3:Jazz/4:Classic/5:Bass
 *****************************************************************************/
void dfplayer_setEqualizer(uint8_t preset) {
  dfplayer_send(DFPLAYER_SET_EQUALIZER, 0, preset);
}

/*****************************************************************************
 * @brief Specify single repeat playback (Tracks 0001-3000/9999)
 * NOTE:
 *	- command does't work when module is paused or stopped
 *	- don’t copy 0003.mp3 & then 0001.mp3, because 0003.mp3 will be played first
 *****************************************************************************/
void dfplayer_repeatTrack(uint16_t track) {
  dfplayer_send(DFPLAYER_REPEATE_TRACK, (track >> 8), track);
}

/*****************************************************************************
 * @brief Specify playback of a device (0/1) (0:USB/1:SD)
 * NOTE:
 *	- source:
 *  	- 1=USB-Disk
 *  	- 2=TF-Card
 *  	- 3=Aux
 *  	- 4=sleep (for YX5200)/NOR-Flash (for GD3200)
 *  	- 5=NOR-Flash
 *  	- 6=Sleep
 *	- source 3..6 may not be supported by some modules!!!
 *
 *	- module automatically detect source if source is on-line
 *	- module automatically enter standby after setting source
 *	- this command interrupt playback!!!
 *	- wait 200ms to select source
 *****************************************************************************/
void dfplayer_setSource(uint8_t source) {
  dfplayer_send(DFPLAYER_SET_SOURCE, 0, source);
}

/*****************************************************************************
 * @brief Set Sleep
 * NOTE:
 * 	- standby not the same as sleep mode
 *  - use "wakeup()" to exit standby
 *  - module does't respond to any playback commands in standby mode, other commands OK
 *  - looks like does nothing, consumption before & after command 24mA
 *****************************************************************************/
void dfplayer_setSleepMode() {
  dfplayer_send(DFPLAYER_SET_SLEEP_MODE, 0, 0);
}

/*****************************************************************************
 * @brief Set Normal mode, N/A (Reserved)
 * NOTE:
 * 	- standby not the same as sleep mode
 *  - use "wakeup()" to exit standby
 *  - module does't respond to any playback commands in standby mode, other commands OK
 *  - looks like does nothing, consumption before & after command 24mA
 *****************************************************************************/
void dfplayer_setNormalMode() {
  dfplayer_send(DFPLAYER_SET_NORMAL_MODE, 0, 0);
}

/*****************************************************************************
 * @brief Reset all settings to factory default
 * NOTE:
 *  - wait for player to boot, 1.5sec..3sec depends on SD-card size
 *****************************************************************************/
void dfplayer_reset() {
  dfplayer_send(DFPLAYER_RESET, 0, 0);
}

/*****************************************************************************
 * @brief Resume playing current track, after pause or stop
 *****************************************************************************/
void dfplayer_play() {
  dfplayer_send(DFPLAYER_PLAY, 0, 0);
}

/*****************************************************************************
 * @brief Pause current track
 *****************************************************************************/
void dfplayer_pause() {
  dfplayer_send(DFPLAYER_PAUSE, 0, 0);
}

/*****************************************************************************
 * @brief Specify playback a track in a folder
 * NOTE:
 *  - folder name must be 01..99
 *  - up to 001..255 songs in each folder
 *****************************************************************************/
void dfplayer_playFolder(uint8_t folder, uint8_t track) {
  dfplayer_send(DFPLAYER_PLAY_FOLDER, folder, track);
}

/*****************************************************************************
 * @brief Audio amplification setting (MSB=1:amplifying on, LSB:set gain 0-31)
 * NOTE:
 *  - HD-byte value, 0x01=enable gain & 0x00=disable gain
 *  - LD-byte value, 0..31=gain
 *
 *  - feature may not be supported by some modules!!!
 *****************************************************************************/
void dfplayer_setDacGain(uint8_t enable, uint8_t gain) {
  dfplayer_send(DFPLAYER_SET_DAC_GAIN, enable, gain);
}

/*****************************************************************************
 * @brief Repeat playback all files in chronological order until it receives stop or pause command
 * NOTE:
 *  - 0x00=stop repeat playback & 0x01=start repeat playback
 *  - command does't work when module is paused or stopped
 *  - any playback command will switch back to normal playback mode
 *
 *  - files in the root must start with 4 decimal digits with leading zeros
 *   - example: SD_ROOT/0001 - My favorite song.mp3
 *  - don’t copy 0003.mp3 & then 0001.mp3, because 0003.mp3 will be played first
 *****************************************************************************/
void dfplayer_repeatAll(uint8_t enable) {
  dfplayer_send(DFPLAYER_REPEAT_ALL, 0, enable);
}

/*****************************************************************************
 * @brief Specify playback of folder named “MP3”
 * NOTE:
 *  - folder name must be "mp3" or "MP3"
 *  - up to 0001..9999 songs in each folder
 *
 *  - files in folder must start with 4 decimal digits with leading zeros
 *   - example: SD_ROOT/mp3/0001 - My favorite song.mp3
 *  - module speed will decrease as the folder gets bigger, place no more than 3000 tracks to keep the speed
 *****************************************************************************/
void dfplayer_playMp3Folder(uint16_t track) {
  dfplayer_send(DFPLAYER_PLAY_MP3_FOLDER, (track >> 8), track);
}

/*****************************************************************************
 * @brief Interrupt current track & play specific track number from "advert" folder, than resume current track
 * NOTE:
 *  - folder name must be "advert" or "ADVERT"
 *  - up to 0001..9999 songs in each folder
 *  - command does't work when module is paused or stopped
 *
 *  - files in folder must start with 4 decimal digits with leading zeros
 *   - example: SD_ROOT/advert/0001 - My favorite song.mp3
 *****************************************************************************/
void dfplayer_playAdvertFolder(uint16_t track) {
  dfplayer_send(DFPLAYER_PLAY_ADVERT_FOLDER, (track >> 8), track);
}

/*****************************************************************************
 * @brief  Play specific track number from folder, if you need more than 256 tracks in a folder
 * NOTE:
 *  - folder name must be 01..15
 *  - up to 0001..3000 songs in each folder
 *  - feature may not be supported by some modules!!!
 *
 *  - files in folder must start with 4 decimal digits with leading zeros
 *   - example: SD_ROOT/01/0001 - My favorite song.mp3
 *****************************************************************************/
void dfplayer_play3000Folder(uint16_t track) {
  dfplayer_send(DFPLAYER_PLAY_3000_FOLDER, (track >> 8), track);
}

/*****************************************************************************
 * @brief Stop interrupting current track while playing track from "advert" folder
 * NOTE:
 *  - see playAdvertFolder() for details
 *****************************************************************************/
void dfplayer_stopAdvertFolder() {
  dfplayer_send(DFPLAYER_STOP_ADVERT_FOLDER, 0, 0);
}

/*****************************************************************************
 * @brief Stop playing current track
 * NOTE:
 *  - always call stop() after pause(), otherwise a new track from another folder won't play
 *****************************************************************************/
void dfplayer_stop() {
  dfplayer_send(DFPLAYER_STOP, 0, 0);
}
