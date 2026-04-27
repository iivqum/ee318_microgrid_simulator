/*
 * display.h
 *
 *  Created on: 26 Feb 2026
 *      Author: Josh
 */

#include <stdint.h>
#include "fsl_gpio.h"
#include "fsl_common_arm.h"
#include "fsl_ctimer.h"

#ifndef DISPLAY_H_
#define DISPLAY_H_

/*
24 bit Address structure

Row select		Colour select

MSB	00000000	000 000 000 0000000 LSB
	01234567	RGB RGB RGB Padding
				C1	C2	C3

Colour bits are active low
Row bits are active high
*/

#define DISPLAY_DATA_LENGTH 24

#define ROWSEL1 (uint32_t)(1 << 23)
#define ROWSEL2 (uint32_t)(1 << 22)
#define ROWSEL3 (uint32_t)(1 << 21)
#define ROWSEL4 (uint32_t)(1 << 20)
#define ROWSEL5 (uint32_t)(1 << 19)
#define ROWSEL6 (uint32_t)(1 << 18)
#define ROWSEL7 (uint32_t)(1 << 17)
#define ROWSEL8 (uint32_t)(1 << 16)

#define C1R (uint32_t)(1 << 15)
#define C1G (uint32_t)(1 << 14)
#define C1B (uint32_t)(1 << 13)

#define C2R (uint32_t)(1 << 12)
#define C2G (uint32_t)(1 << 11)
#define C2B (uint32_t)(1 << 10)

#define C3R (uint32_t)(1 << 9)
#define C3G (uint32_t)(1 << 8)
#define C3B (uint32_t)(1 << 7)

#define PADDING_MASK (uint32_t)0x7f

/*
00000000 111 111 111 0000000
Disable all row and column drivers by default
*/
#define DISPLAY_REST (uint32_t)0xff80

#define DISPLAY_CLOCK 14
#define DISPLAY_DATA 16
#define DISPLAY_OEN  18
#define DISPLAY_R_CLOCK 6

#define DISPLAY_OVERSAMPLE 32

typedef struct {
	uint8_t r, g, b;
} rgb_led;

void display_callback(uint32_t);
void set_rgb0(uint8_t, uint8_t, uint8_t);
void set_rgb1(uint8_t, uint8_t, uint8_t);
void display_write_byte(uint8_t);
void display_write_bytes(uint8_t *, uint16_t);
void display_clock_store();
void display_clock_output();
void display_write_int(uint32_t);
void display_enable();
void display_disable();
void display_test();
void display_reset();
void display_set_led();

#endif /* DISPLAY_H_ */
