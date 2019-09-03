/*
	Based on https://os.mbed.com/users/eggsylah/code/SSD1306-I2C/
*/
#ifndef SSD1306_H_
#define SSD1306_H_

#include "Adafruit_GFX.h"
#include <mbed.h>
#include <stdint.h>

#define SSD1306_LCDWIDTH                  128
#define SSD1306_LCDHEIGHT                 64

#define BLACK 0
#define WHITE 1
#define INVERSE 2

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA

#define SSD1306_SETVCOMDETECT 0xDB

#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9

#define SSD1306_SETMULTIPLEX 0xA8

#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10

#define SSD1306_SETSTARTLINE 0x40

#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22

#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8

#define SSD1306_SEGREMAP 0xA0

#define SSD1306_CHARGEPUMP 0x8D

// Scrolling #defines
#define SSD1306_ACTIVATE_SCROLL 0x2F
#define SSD1306_DEACTIVATE_SCROLL 0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A


class SSD1306 : public Adafruit_GFX 
{
	I2C * _i2c;
  public:
	SSD1306(I2C * i2c, int16_t w=SSD1306_LCDWIDTH, int16_t h=SSD1306_LCDHEIGHT);

	void drawPixel(int16_t x, int16_t y, uint16_t color);

	void hw_setup();

	void begin(bool reset=true);
	void display(void);

	void clearDisplay(void);
	void invertDisplay(uint8_t i);
	void startscrollleft(uint8_t start, uint8_t stop);
	void startscrollright(uint8_t start, uint8_t stop);
	void startscrolldiagright(uint8_t start, uint8_t stop);
	void startscrolldiagleft(uint8_t start, uint8_t stop);
	void stopscroll(void);
	void dim(bool dim);
	void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
	void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);


  protected:
  int16_t
     _width, _height; // Display w/h as modified by current rotation

   uint8_t
     textsize,
     rotation;

  private:
	void _sendData(const uint8_t *blk, uint32_t len, bool isData);
	void sendCommands(const uint8_t *blk, uint32_t len);
	void sendData(const uint8_t *blk, uint32_t len);

	//void ssd1306_command(uint8_t c);

	void drawFastHLineInternal(int16_t x, int16_t y, int16_t w, uint16_t color);
	void drawFastVLineInternal(int16_t x, int16_t __y, int16_t __h, uint16_t color);

	void _scroll(uint8_t mode, uint8_t start, uint8_t stop);
};



#endif /* SOURCE_SSD1306_LIBARY_H_ */
