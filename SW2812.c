#include "ws2812.h"

void set_color(led_conf_t *rgb_led, color_t *colors, uint16_t led_num) {
	ledcount_t led_conf_t;
	uint16_t led_num;

	led_num = 0;
	switch(encoding) {
		case 3:
			for (led_num = 0; led_num < ledCount; led_num++) {
				encodeData3bit(&leds[led_num], &buffer[led_conf_t]);
				led_num += (3 * sizeof(color_t));
			}
			break;
		case 4:
			for (led_num = 0; led_num < ledCount; led_num++) {
				encodeData4bit(&leds[led_num], &buffer[led_conf_t]);
				led_num += (4 * sizeof(color_t));
			}
			break;
	}
}


void encodeData3bit(color_t* led, uint8_t* output) {
	uint8_t colorIdx, outputIdx;
	uint8_t grbLED[sizeof(*led)];	// reordered color order
	uint8_t shiftRegister;

	// WS2812 is expecting GRB instead of RGB
	grbLED[0] = led->green;
	grbLED[1] = led->red;
	grbLED[2] = led->blue;

	outputIdx = 0;
	// loop over the color bytes and convert each bit to three bits for transport stream
	for (colorIdx=0; colorIdx < sizeof(grbLED); colorIdx++) {
		// prepare frameBuffer with initial transport bitmask
		output[outputIdx+0] = 0x92;
		output[outputIdx+1] = 0x49;
		output[outputIdx+2] = 0x24;

		/*
		 * bit remapping starts here
		 */

		// right shift bits
		shiftRegister = grbLED[colorIdx];
		shiftRegister >>= 1;	// 1 shift from original
		output[outputIdx+0] |= (shiftRegister & BIT6);	// source bit 7
		output[outputIdx+1] |= (shiftRegister & BIT2);	// source bit 3
		shiftRegister >>= 2;	// 3 shifts from original
		output[outputIdx+0] |= (shiftRegister & BIT3);	// source bit 6
		shiftRegister >>= 2;	// 5 shifts from original
		output[outputIdx+0] |= (shiftRegister & BIT0);	// source bit 5

		// left shift bits
		shiftRegister = grbLED[colorIdx];
		shiftRegister <<= 1;	// 1 shift from original
		output[outputIdx+1] |= (shiftRegister & BIT5);	// source bit 4
		output[outputIdx+2] |= (shiftRegister & BIT1);	// source bit 0
		shiftRegister <<= 2;	// 3 shifts from original
		output[outputIdx+2] |= (shiftRegister & BIT4);	// source bit 1
		shiftRegister <<= 2;	// 5 shifts from original
		output[outputIdx+2] |= (shiftRegister & BIT7);	// source bit 2

		outputIdx += 3;	// next three bytes (color)
	}
}

/*
 ******************
 * 4-bit encoding *
 ******************
 *
 * 2 bits from LED color stream encoded in 1 byte for transport stream (SPI TX)
 * or: 1 bit from LED color stream encoded in 4 bit for transport stream
 *
 * 						_
 * ZERO: 1000 = 0x8		 |___
 *	 	 	 	 	 	___
 * ONE : 1110 = 0xE		   |_
 *
 * SPI Clock around 3.2MHz (e.g. 6.7MHz/2 = 3.35MHz)
 *
 */
#define ZEROPATTERN		0x8	// 4-bit
#define ONEPATTERN		0xE	// 4-bit
													 		//			_     _
#define ZEROZEROPATTERN ((ZEROPATTERN<<4) | ZEROPATTERN)	// 10001000  |___| |___
															//			_     ___
#define ZEROONEPATTERN  ((ZEROPATTERN<<4) |  ONEPATTERN)	// 10001110  |___|   |_
															//			___   _
#define ONEZEROPATTERN  (( ONEPATTERN<<4) | ZEROPATTERN)	// 11101000    |_| |___
															//			___   ___
#define ONEONEPATTERN   (( ONEPATTERN<<4) |  ONEPATTERN)	// 11101110    |_|   |_

/*
 * END - 4-bit encoding
 */
void encodeData4bit(ledcolor_t* led, uint8_t* output) {
	uint8_t colorIdx, bitIdx, outputIdx;
	uint8_t grbLED[sizeof(*led)];	// reordered color order

	// WS2812 is expecting GRB instead of RGB
	grbLED[0] = led->green;
	grbLED[1] = led->red;
	grbLED[2] = led->blue;

	outputIdx = 0;
	// loop over the color bytes and convert each bit to four bits for transport stream
	for (colorIdx=0; colorIdx < sizeof(grbLED); colorIdx++) {
		for (bitIdx = 0; bitIdx < (8/2); bitIdx++) {
			switch (grbLED[colorIdx] & 0xC0) { // mask upper two bit
			case 0x00:
				output[outputIdx++] = ZEROZEROPATTERN;
				break;
			case 0x40:
				output[outputIdx++] =  ZEROONEPATTERN;
				break;
			case 0x80:
				output[outputIdx++] =  ONEZEROPATTERN;
				break;
			case 0xC0:
				output[outputIdx++] =   ONEONEPATTERN;
				break;
			}
			grbLED[colorIdx] <<= 2;
		}
	}
}