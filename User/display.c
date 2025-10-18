#include <stdio.h>
#include <ch32v00x.h>
#include "display.h"

/**
 * Send data to display
 */
void display_send(uint8_t command, uint8_t *data, uint8_t size) {
  while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET) {
    //printf("I2C_FLAG_BUSY\r\n");
  }
  
  I2C_GenerateSTART(I2C1, ENABLE);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) {
    //printf("I2C_EVENT_MASTER_MODE_SELECT\r\n");
  }

  I2C_Send7bitAddress(I2C1, DISPLAY_I2C_ADDRESS, I2C_Direction_Transmitter);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
    //printf("I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED\r\n");
  }

  I2C_SendData(I2C1, command);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
    //printf("I2C_EVENT_MASTER_BYTE_TRANSMITTED\r\n");
  }

  for (uint8_t p = 0; p < size; p++) {
    I2C_SendData(I2C1, *data);
    data++;

    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
      //printf("I2C_EVENT_MASTER_BYTE_TRANSMITTED\r\n");
    }
  }
  
  I2C_GenerateSTOP(I2C1, ENABLE);
}

/**
 * Send byte to display
 */
void display_sendCommand(uint8_t command) {
  display_send(0, &command, 1);
}

/**
 * Initialization
 */
void display_init() {
  // Set MUX Ratio A8h, 3Fh
  // Set Display Offset D3h, 00h
  // Set Display Start Line 40h
  // Set Segment re-map A0h/A1h
  // Set COM Output Scan Direction C0h/C8h
  // Set COM Pins hardware configuration DAh, 02
  // Set Contrast Control 81h, 7Fh
  // Disable Entire Display On A4h
  // Set Normal Display A6h
  // Set Osc Frequency D5h, 80h
  // Enable charge pump regulator 8Dh, 14h
  // Display On AFh

  Delay_Ms(100);

  display_sendCommand(SSD1306_DISPLAY_OFF);

	display_sendCommand(SSD1306_SET_DISPLAY_CLOCK_DIV);
  display_sendCommand(0x00);
  
  display_sendCommand(SSD1306_SET_MULTIPLEX);
  display_sendCommand(SSD1306_MULTIPLEX_128_32);

  display_sendCommand(SSD1306_SET_DISPLAY_OFFSET);
  display_sendCommand(0x00);

  display_sendCommand(SSD1306_SET_START_LINE | 0x00);

  display_sendCommand(SSD1306_CHARGE_PUMP);
  display_sendCommand(0x14); // Enable Charge Pump

  display_sendCommand(SSD1306_MEMORY_MODE);
  display_sendCommand(0x00); // Horizontal addressing mode (A[1:0]=00b)
  
  display_sendCommand(SSD1306_SEG_REMAP_YES);
	
  display_sendCommand(SSD1306_COM_SCAN_DEC); // Flip?

	display_sendCommand(SSD1306_SET_COM_PINS);
  display_sendCommand(0x02); //for 128x32 0x02, for 128x64 0x12;

  display_sendCommand(SSD1306_DEACTIVATE_SCROLL);

  display_sendCommand(SSD1306_COLUMN_ADDR);
  display_sendCommand(0x00);
  display_sendCommand(0xFF);

  display_sendCommand(SSD1306_PAGE_ADDR);
  display_sendCommand(0x00);
  display_sendCommand(0x07);

	display_sendCommand(SSD1306_SET_CONTRAST);
	display_sendCommand(DISPLAY_DEFAULT_CONTRAST);

	//display_sendCommand(SSD1306_SET_PRE_CHARGE);
	//display_sendCommand(0xF1);

	display_sendCommand(SSD1306_SET_V_COM_DETECT);
	display_sendCommand(0x40);

	display_sendCommand(SSD1306_DISPLAY_ALLON_RESUME);

	display_sendCommand(SSD1306_DISPLAY_ON);
}

/**
 * @brief Set Cursor
 */
void display_setCursor(uint8_t x, uint8_t y) {
	// Y - 1 unit = 1 page (8 pixel rows)
	// X - 1 unit = 8 pixel columns

	display_sendCommand(0x00 | (8 * x & 0x0F)); 		      // Set column lower address
	display_sendCommand(0x10 | ((8 * x >> 4) & 0x0F));   // Set column higher address
	display_sendCommand(0xB0 | y);                       // Set page address
}

/**
 * @brief Send data to display
 */
void display_sendData(uint8_t page, uint8_t *data, uint8_t size) {
	//display_sendCommand(SSD1306_PAGE_ADDR);
	//display_sendCommand(0x00);
	//display_sendCommand(0x07);

	display_sendCommand(SSD1306_SET_PAGE_START_ADDRESS | page);
	display_sendCommand(SSD1306_SET_HIGHER_COLUMN_START_ADDRESS);
	display_sendCommand(SSD1306_SET_LOWER_COLUMN_START_ADDRESS);

	display_send(SSD1306_SET_START_LINE, data, size);
}

/**
 * @brief Set Contrast, but it look's like does not work
 */
void display_setContrast(uint8_t value) {
	display_sendCommand(SSD1306_SET_CONTRAST);
	display_sendCommand(value);
}
