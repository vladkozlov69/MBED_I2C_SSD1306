#include "mbed.h"

#include "SSD1306.h"

extern Serial pc;

extern "C" {
}

#define NUM_ELEMENTS(x)	((sizeof x)/(sizeof x[0]))


//I2C i2c(I2C_SDA, I2C_SCL);
//DigitalOut rst(PTD1); //D13);			//reset pin on D13


// the memory buffer for the LCD
static uint8_t buffer[SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH / 8];

volatile uint8_t dma_pause = 0;

SSD1306::SSD1306(I2C * i2c, int16_t w, int16_t h) : Adafruit_GFX(w, h) 
{
	this->_i2c = i2c;
}

#define ssd1306_swap(a, b) { int16_t t = a; a = b; b = t; }

void SSD1306::hw_setup() 
{
	_i2c->frequency(400000);
}


// the most basic function, set a single pixel
void SSD1306::drawPixel(int16_t x, int16_t y, uint16_t color) {
	if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
		return;

	// check rotation, move pixel around if necessary
	switch (rotation) {
	case 1:
		ssd1306_swap(x, y);
		x = WIDTH - x - 1;
		break;
	case 2:
		x = WIDTH - x - 1;
		y = HEIGHT - y - 1;
		break;
	case 3:
		ssd1306_swap(x, y);
		y = HEIGHT - y - 1;
		break;
	}

	// x is which column
	uint8_t *p = &buffer[x + (y/8)*SSD1306_LCDWIDTH];
	uint8_t v = 1 << (y & 7);

	switch (color) {
	case WHITE:
		*p |= v;
		break;
	case BLACK:
		*p &= ~v;
		break;
	case INVERSE:
		*p ^= v;
		break;
	}

}

void SSD1306::_sendData(const uint8_t *blk, uint32_t len, bool isData) {
    const uint8_t *p = blk;

	//pc.printf("SendData...\r\n");
    // now send the data
    uint8_t control = 0x00 | (isData ? 0x40 : 0x00);

 
    if (isData) {
		_i2c->start();
		//pc.printf("%0.2x ", *p);
	    _i2c->write(0x3C<<1);
	    
	    //control |= 0x80;
	    _i2c->write(control);
		
	    for (uint32_t i=0; i<len; i++, p++) {
		    //pc.printf("%0.2x ", *p);
	        int error = _i2c->write(*p);
	        //int error = 1;
	        //wait(0.1);
	        if (error != 1)
	            pc.printf("I2C error: %d\r\n", error);
	    }
	    _i2c->stop();

    } else {
	    for (uint32_t i=0; i<len; i++, p++) {
		    _i2c->start();
		    //pc.printf("%0.2x ", *p);
	        _i2c->write(0x3C<<1);
	        _i2c->write(control);
	        int error = _i2c->write(*p);
	        //int error = 1;
	        //wait(0.1);
	        _i2c->stop();
	        if (error != 1)
	            pc.printf("I2C error: %d\r\n", error);
	    }
    }
}

void SSD1306::sendCommands(const uint8_t *blk, uint32_t len) {
	_sendData(blk, len, false);
}

void SSD1306::sendData(const uint8_t *blk, uint32_t len) {
	_sendData(blk, len, true);
}

void SSD1306::begin(bool reset) {
    if (reset) {		//pulse the reset pin -- maybe replace with RC network
    	//rst = 1;
    	// wait_ms(1);
    	// rst = 0;
    	// wait_ms(10);
    	// rst = 1;	
    }
    	
	const uint8_t cmds[] = {
		SSD1306_DISPLAYOFF,
		SSD1306_SETDISPLAYCLOCKDIV,
		0x80,                                   // the suggested ratio 0x80
		SSD1306_SETMULTIPLEX,
		SSD1306_LCDHEIGHT - 1,
		SSD1306_SETDISPLAYOFFSET,
		0x0,                                    // no offset
		SSD1306_SETSTARTLINE | 0x0,             // line #0
		SSD1306_CHARGEPUMP,
		0x14,
		SSD1306_MEMORYMODE,
		0x00,                                   // 0x0 act like ks0108
		SSD1306_SEGREMAP | 0x1,
		SSD1306_COMSCANDEC,
		SSD1306_SETCOMPINS,
		0x12,
		SSD1306_SETCONTRAST,
		0xCF,
		SSD1306_SETPRECHARGE,
		0xF1,
		SSD1306_SETVCOMDETECT,
		0x40,
		SSD1306_DISPLAYALLON_RESUME,
		SSD1306_NORMALDISPLAY,
		SSD1306_DEACTIVATE_SCROLL,
		SSD1306_DISPLAYON                     //--turn on oled panel
	};

        sendCommands(cmds, NUM_ELEMENTS(cmds));

}


void SSD1306::display(void) {
	const uint8_t cmds[] = {
		SSD1306_COLUMNADDR,
		0,   				// Column start address (0 = reset)
		SSD1306_LCDWIDTH - 1,		// Column end address (127 = reset)
		SSD1306_PAGEADDR,
		0,				 // Page start address (0 = reset)
		7				 // Page end address
		};

    sendCommands(cmds, NUM_ELEMENTS(cmds));
	//now send the screen image
	sendData(buffer, NUM_ELEMENTS(buffer));
}

void SSD1306::invertDisplay(uint8_t i) {
	const uint8_t normalCmd[] = { SSD1306_NORMALDISPLAY };
	const uint8_t invertCmd[] = { SSD1306_INVERTDISPLAY };
        sendCommands( i ? invertCmd : normalCmd, 1);
}


void SSD1306::_scroll(uint8_t mode, uint8_t start, uint8_t stop) {
	uint8_t cmds[] = { mode, 0, start, 0, stop, 0, 0xFF, SSD1306_ACTIVATE_SCROLL };
        sendCommands(cmds, NUM_ELEMENTS(cmds));
}
// startscrollright
// Activate a right handed scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void SSD1306::startscrollright(uint8_t start, uint8_t stop) {
	_scroll(SSD1306_RIGHT_HORIZONTAL_SCROLL, start, stop);
}

// startscrollleft
// Activate a right handed scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void SSD1306::startscrollleft(uint8_t start, uint8_t stop) {
	_scroll(SSD1306_LEFT_HORIZONTAL_SCROLL, start, stop);
}

// startscrolldiagright
// Activate a diagonal scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void SSD1306::startscrolldiagright(uint8_t start, uint8_t stop) {
    uint8_t cmds[] = { 
	SSD1306_SET_VERTICAL_SCROLL_AREA,
	0X00,
	SSD1306_LCDHEIGHT,
	SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL,
	0X00,
	start,
	0X00,
	stop,
	0X01,
	SSD1306_ACTIVATE_SCROLL
    };

    sendCommands(cmds, NUM_ELEMENTS(cmds));
}

// startscrolldiagleft
// Activate a diagonal scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void SSD1306::startscrolldiagleft(uint8_t start, uint8_t stop) {
    uint8_t cmds[] = { 
	SSD1306_SET_VERTICAL_SCROLL_AREA,
	0X00,
	SSD1306_LCDHEIGHT,
	SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL,
	0X00,
	start,
	0X00,
	stop,
	0X01,
	SSD1306_ACTIVATE_SCROLL
    };

    sendCommands(cmds, NUM_ELEMENTS(cmds));
}


void SSD1306::stopscroll(void) {
	const uint8_t cmds[] = { SSD1306_DEACTIVATE_SCROLL };
        sendCommands(cmds, NUM_ELEMENTS(cmds));
}

// Dim the display
// dim = true: display is dimmed
// dim = false: display is normal
void SSD1306::dim(bool dim) 
{
	// the range of contrast to too small to be really useful
	// it is useful to dim the display
	uint8_t cmds[] = {SSD1306_SETCONTRAST, dim ? (uint8_t)0 : (uint8_t)0xCF};
	sendCommands(cmds, NUM_ELEMENTS(cmds));
}

// clear everything
void SSD1306::clearDisplay(void) {
	memset(buffer, 0, (SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8));
}

#if 1
void SSD1306::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
	bool bSwap = false;
	switch (rotation) {
	case 0:
		// 0 degree rotation, do nothing
		break;
	case 1:
		// 90 degree rotation, swap x & y for rotation, then invert x
		bSwap = true;
		ssd1306_swap(x, y);
		x = WIDTH - x - 1;
		break;
	case 2:
		// 180 degree rotation, invert x and y - then shift y around for height.
		x = WIDTH - x - 1;
		y = HEIGHT - y - 1;
		x -= (w - 1);
		break;
	case 3:
		// 270 degree rotation, swap x & y for rotation, then invert y  and adjust y for w (not to become h)
		bSwap = true;
		ssd1306_swap(x, y);
		y = HEIGHT - y - 1;
		y -= (w - 1);
		break;
	}

	if (bSwap) {
		drawFastVLineInternal(x, y, w, color);
	} else {
		drawFastHLineInternal(x, y, w, color);
	}
}
#endif


void SSD1306::drawFastHLineInternal(int16_t x, int16_t y, int16_t w,
		uint16_t color) {
	// Do bounds/limit checks
	if (y < 0 || y >= HEIGHT) {
		return;
	}

	// make sure we don't try to draw below 0
	if (x < 0) {
		w += x;
		x = 0;
	}

	// make sure we don't go off the edge of the display
	if ((x + w) > WIDTH) {
		w = (WIDTH - x);
	}

	// if our width is now negative, punt
	if (w <= 0) {
		return;
	}

	// set up the pointer for  movement through the buffer
	register uint8_t *pBuf = buffer;
	// adjust the buffer pointer for the current row
	pBuf += ((y / 8) * SSD1306_LCDWIDTH);
	// and offset x columns in
	pBuf += x;

	register uint8_t mask = 1 << (y & 7);

	switch (color) {
	case WHITE:
		while (w--)
			*pBuf++ |= mask;
		break;
	case BLACK:
		mask = ~mask;
		while (w--)
			*pBuf++ &= mask;
		break;
	case INVERSE:
		while (w--)
			*pBuf++ ^= mask;
		break;
	}
}

void SSD1306::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
	bool bSwap = false;
	switch (rotation) {
	case 0:
		break;
	case 1:
		// 90 degree rotation, swap x & y for rotation, then invert x and adjust x for h (now to become w)
		bSwap = true;
		ssd1306_swap(x, y)
		x = WIDTH - x - 1;
		x -= (h - 1);
		break;
	case 2:
		// 180 degree rotation, invert x and y - then shift y around for height.
		x = WIDTH - x - 1;
		y = HEIGHT - y - 1;
		y -= (h - 1);
		break;
	case 3:
		// 270 degree rotation, swap x & y for rotation, then invert y
		bSwap = true;
		ssd1306_swap(x, y)
		y = HEIGHT - y - 1;
		break;
	}

	if (bSwap) {
		drawFastHLineInternal(x, y, h, color);
	} else {
		drawFastVLineInternal(x, y, h, color);
	}
}

void SSD1306::drawFastVLineInternal(int16_t x, int16_t __y, int16_t __h, uint16_t color) {

	// do nothing if we're off the left or right side of the screen
	if (x < 0 || x >= WIDTH) {
		return;
	}

	// make sure we don't try to draw below 0
	if (__y < 0) {
		// __y is negative, this will subtract enough from __h to account for __y being 0
		__h += __y;
		__y = 0;

	}

	// make sure we don't go past the height of the display
	if ((__y + __h) > HEIGHT) {
		__h = (HEIGHT - __y);
	}

	// if our height is now negative, punt
	if (__h <= 0) {
		return;
	}

	// this display doesn't need ints for coordinates, use local byte registers for faster juggling
	register uint8_t y = __y;
	register uint8_t h = __h;

	// set up the pointer for fast movement through the buffer
	register uint8_t *pBuf = buffer;
	// adjust the buffer pointer for the current row
	pBuf += ((y / 8) * SSD1306_LCDWIDTH);
	// and offset x columns in
	pBuf += x;

	// do the first partial byte, if necessary - this requires some masking
	register uint8_t mod = (y & 7);
	if (mod) {
		// mask off the high n bits we want to set
		mod = 8 - mod;

		// note - lookup table results in a nearly 10% performance improvement in fill* functions
		// register uint8_t mask = ~(0xFF >> (mod));
		static uint8_t premask[8] = { 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC,
				0xFE };
		register uint8_t mask = premask[mod];

		// adjust the mask if we're not going to reach the end of this byte
		if (h < mod) {
			mask &= (0XFF >> (mod - h));
		}

		switch (color) {
		case WHITE:
			*pBuf |= mask;
			break;
		case BLACK:
			*pBuf &= ~mask;
			break;
		case INVERSE:
			*pBuf ^= mask;
			break;
		}

		// fast exit if we're done here!
		if (h < mod) {
			return;
		}

		h -= mod;

		pBuf += SSD1306_LCDWIDTH;
	}

	// write solid bytes while we can - effectively doing 8 rows at a time
	if (h >= 8) {
		if (color == INVERSE) { // separate copy of the code so we don't impact performance of the black/white write version with an extra comparison per loop
			do {
				*pBuf = ~(*pBuf);

				// adjust the buffer forward 8 rows worth of data
				pBuf += SSD1306_LCDWIDTH;

				// adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
				h -= 8;
			} while (h >= 8);
		} else {
			// store a local value to work with
			register uint8_t val = (color == WHITE) ? 255 : 0;

			do {
				// write our value in
				*pBuf = val;

				// adjust the buffer forward 8 rows worth of data
				pBuf += SSD1306_LCDWIDTH;

				// adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
				h -= 8;
			} while (h >= 8);
		}
	}

	// now do the final partial byte, if necessary
	if (h) {
		mod = h & 7;
		// this time we want to mask the low bits of the byte, vs the high bits we did above
		// register uint8_t mask = (1 << mod) - 1;
		// note - lookup table results in a nearly 10% performance improvement in fill* functions
		static uint8_t postmask[8] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F,
				0x7F };
		register uint8_t mask = postmask[mod];
		switch (color) {
		case WHITE:
			*pBuf |= mask;
			break;
		case BLACK:
			*pBuf &= ~mask;
			break;
		case INVERSE:
			*pBuf ^= mask;
			break;
		}
	}
}
