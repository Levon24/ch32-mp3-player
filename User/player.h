/**
 * @brief Original here https://github.com/enjoyneering/DFPlayer/blob/main/src/DFPlayer.h
 * It just converted to classic C Style
 */

#ifndef _PLAYER_H
#define _PLAYER_H

#ifdef __cplusplus
extern "C" {
#endif

/* UART frame values */
#define PLAYER_UART_FRAME_SIZE      0x0A // Total number of bytes in UART packet, same for cmd & feedback
#define PLAYER_UART_START_BYTE      0x7E // Start byte
#define PLAYER_UART_VERSION         0xFF // Protocol version
#define PLAYER_UART_DATA_LEN        0x06 // Number of data bytes, except start byte, checksum & end byte
#define PLAYER_UART_END_BYTE        0xEF // End byte

/* command controls */
#define PLAYER_PLAY_NEXT            0x01 // Play next uploaded file
#define PLAYER_PLAY_PREVIOUS        0x02 // Play previous uploaded file
#define PLAYER_PLAY_TRACK           0x03 // Play tracks in chronological order, by upload time
#define PLAYER_SET_VOLUME_UP        0x04 // Increment volume by 1
#define PLAYER_SET_VOLUME_DOWN      0x05 // Decrement volume by 1
#define PLAYER_SET_VOLUME           0x06 // Set volume, range 0..30
#define PLAYER_SET_EQUALIZER        0x07 // 0=Off, 1=Pop, 2=Rock, 3=Jazz, 4=Classic, 5=Bass (may not be supported by some modules)
#define PLAYER_REPEATE_TRACK        0x08 // Playing & looping track number 0001..9999
#define PLAYER_SET_SOURCE    				0x09 // 1=USB-Disk, 2=TF-Card, 3=Aux, 4=sleep(YX5200)/NOR-Flash(GD3200), 5=NOR-Flash, 6=Sleep (3..6 may not be supported by some modules)
#define PLAYER_SET_SLEEP_MODE       0x0A // Put player in standby mode, not the same as sleep mode
#define PLAYER_SET_NORMAL_MODE      0x0B // Pull player out of standby mode (may not be supported by some modules)
#define PLAYER_RESET                0x0C // Reset module, set all settings to factory default
#define PLAYER_PLAY                 0x0D // Resume playing current track
#define PLAYER_PAUSE                0x0E // Pause playing current track
#define PLAYER_PLAY_FOLDER          0x0F // Play track number 1..255 from folder number 1..99
#define PLAYER_SET_DAC_GAIN         0x10 // Set DAC output gain/output voltage swing (may not be supported by some modules)
#define PLAYER_REPEAT_ALL           0x11 // Repeat playback all files in chronological order
#define PLAYER_PLAY_MP3_FOLDER      0x12 // Play track number 0001..9999 from "mp3" folder 
#define PLAYER_PLAY_ADVERT_FOLDER   0x13 // Interrupt current track & play track number 0001..9999 from "advert" folder, than resume current track
#define PLAYER_PLAY_3000_FOLDER     0x14 // Play track number 0001..3000 from folder that supports 3000 tracks (may not be supported by some modules)
#define PLAYER_STOP_ADVERT_FOLDER   0x15 // Stop interrupting current track while playing track from "advert" folder
#define PLAYER_STOP                 0x16 // Stop playing current track
#define PLAYER_REPEAT_FOLDER        0x17 // Repeat playback folder number 01..99
#define PLAYER_RANDOM_ALL_FILES     0x18 // Play all tracks in random order
#define PLAYER_LOOP_CURRENT_TRACK   0x19 // Loop currently played track
#define PLAYER_SET_DAC              0x1A // Enable/disable DAC (mute/unmute), 0=enable, 1=disable
#define PLAYER_PLAY_ADVERT_FOLDER_N 0x25 // Interrupt current track & play track number 001..255 from "advert1".."advert9" folder, than resume current track (may not be supported by some modules)

/* request command controls */
#define PLAYER_RETURN_CODE_DONE     0x3D // Track playback is is completed, module return this status automatically after the track has been played
#define PLAYER_RETURN_CODE_READY    0x3F // Ready after boot or reset, module return this status automatically after boot or reset
#define PLAYER_RETURN_ERROR         0x40 // Error, module return this status automatically if command is not accepted (details located in 7-th RX byte)
#define PLAYER_RETURN_CODE_OK_ACK   0x41 // OK, command is accepted (returned only if ACK/feedback byte is set to 0x01)
#define PLAYER_GET_STATUS           0x42 // Get current stutus, see NOTE
#define PLAYER_GET_VOL              0x43 // Get current volume, range 0..30
#define PLAYER_GET_EQ               0x44 // Get current EQ, 0=Off, 1=Pop, 2=Rock, 3=Jazz, 4=Classic, 5=Bass (may not be supported by some modules)
#define PLAYER_GET_PLAY_MODE        0x45 // Get current loop mode, 0=loop all, 1=loop folder, 2=loop track, 3=random, 4=disable (may not be supported by some modules)
#define PLAYER_GET_VERSION          0x46 // Get software version
#define PLAYER_GET_QNT_USB_FILES    0x47 // Get total number of tracks on USB-Disk
#define PLAYER_GET_QNT_TF_FILES     0x48 // Get total number of tracks on TF-card
#define PLAYER_GET_QNT_FLASH_FILES  0x49 // Get total number of tracks on NOR-Flash (may not be supported by some modules)
#define PLAYER_GET_USB_TRACK        0x4B // Get currently playing track number on USB-Disk
#define PLAYER_GET_TF_TRACK         0x4C // Get currently playing track number on TF-card
#define PLAYER_GET_FLASH_TRACK      0x4D // Get currently playing track number on NOR-Flash (may not be supported by some modules)
#define PLAYER_GET_QNT_FOLDER_FILES 0x4E // Get total number of tracks in folder
#define PLAYER_GET_QNT_FOLDERS      0x4F // Get total number of folders in current source (may not be supported by some modules)

/* misc */
#define PLAYER_BOOT_DELAY           3000 // Average player boot time 1500sec..3000msec, depends on SD-card size
#define PLAYER_CMD_DELAY            350  // Average read command timeout 200msec..300msec for YX5200/AAxxxx chip & 350msec..500msec for GD3200B/MH2024K chip

/* List of supported modules */
enum player_module {
  PLAYER_MINI,				// DFPlayer Mini, MP3-TF-16P, FN-M16P (YX5200 chip, YX5300 chip or JL AAxxxx chip from Jieli)
  PLAYER_FN_X10P,			// FN-M10P, FN-S10P (FN6100 chip)
  PLAYER_HW_247A,			// DFPlayer Mini HW-247A (GD3200B chip)
  PLAYER_NO_CHECKSUM  // no checksum calculation, not recomended for MCU without external crystal oscillator
};

/* Callback */
enum player_callback {
  PLAYER_CALLBACK_UNDEFINED,
  PLAYER_CALLBACK_TRACK,
};

/* Commands */
void player_playNext();
void player_playPrevious();
void player_playTrack(uint16_t track);
void player_volumeUp();
void player_volumeDown();
void player_setVolume(uint8_t volume);
void player_setEqualizer(uint8_t preset);
void player_repeatTrack(uint16_t track);
void player_setSource(uint8_t source);
void player_setSleepMode();
void player_setNormalMode();
void player_reset();
void player_play();
void player_pause();
void player_playFolder(uint8_t folder, uint8_t track);
void player_setDacGain(uint8_t enable, uint8_t gain);
void player_repeatAll(uint8_t enable);
void player_playMp3Folder(uint16_t track);
void player_playAdvertFolder(uint16_t track);
void player_play3000Folder(uint16_t track);
void player_stopAdvertFolder();
void player_stop();
void player_repeatFolder(uint8_t folder);
void player_randomAll();
void player_repeatCurrentTrack(uint8_t repeat);
void player_enableDac(uint8_t enable);
void player_return();

#ifdef __cplusplus
}
#endif

#endif
