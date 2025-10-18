#ifndef _DISPLAY_H
#define _DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#define DISPLAY_WIDTH    128	// 128 Pixels
#define DISPLAY_HEIGHT   32   // 32  Pixels

#define DISPLAY_I2C_SPEED    400000
#define DISPLAY_I2C_ADDRESS  0x78
#define DISPLAY_I2C_WAIT     250

#define DISPLAY_DEFAULT_CONTRAST  0x7F

// commands
#define SSD1306_DISPLAY_OFF                     0xAE
#define SSD1306_DISPLAY_ON                      0xAF
#define SSD1306_SET_DISPLAY_CLOCK_DIV           0xD5
#define SSD1306_SET_MULTIPLEX                   0xA8
#define SSD1306_MULTIPLEX_128_32                0x1F
#define SSD1306_SET_DISPLAY_OFFSET              0xD3
#define SSD1306_SET_START_LINE                  0x40
#define SSD1306_CHARGE_PUMP                     0x8D
#define SSD1306_MEMORY_MODE                     0x20
#define SSD1306_SEG_REMAP_NO                    0xA0
#define SSD1306_SEG_REMAP_YES                   0xA1
#define SSD1306_COM_SCAN_INC                    0xC0
#define SSD1306_COM_SCAN_DEC                    0xC8
#define SSD1306_SET_COM_PINS                    0xDA
#define SSD1306_COLUMN_ADDR                     0x21
#define SSD1306_PAGE_ADDR                       0x22
#define SSD1306_SET_CONTRAST                    0x81
#define SSD1306_SET_PRE_CHARGE                  0xD9
#define SSD1306_SET_V_COM_DETECT                0xDB
#define SSD1306_DISPLAY_ALLON_RESUME            0xA4
#define SSD1306_SET_PAGE_START_ADDRESS          0xB0
#define SSD1306_SET_HIGHER_COLUMN_START_ADDRESS 0x10
#define SSD1306_SET_LOWER_COLUMN_START_ADDRESS  0x00

// Scrolling defines
#define SSD1306_ACTIVATE_SCROLL             0x2F
#define SSD1306_DEACTIVATE_SCROLL           0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA    0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL     0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL      0x27
#define SSD1306_VERT_AND_RIGHT_HORIZ_SCROLL 0x29
#define SSD1306_VERT_AND_LEFT_HORIZ_SCROLL  0x2A

#define DISPLAY_CMD_DISPLAY_ON			  0xAF
#define DISPLAY_CMD_NORMAL_DISPLAY		0xA6
#define DISPLAY_CMD_INVERSE_DISPLAY	  0xA7

enum _state {
  show_chart, 
  show_settings
};

enum _setup {
  setup_min_moisture,
  setup_contrast
};

void display_init();
void display_send(uint8_t command, uint8_t *data, uint8_t size);
void display_setCursor(uint8_t x, uint8_t y);
void display_sendData(uint8_t page,uint8_t *data, uint8_t size);
void display_setContrast(uint8_t value);

#ifdef __cplusplus
}
#endif

#endif
