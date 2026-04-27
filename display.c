/*
 * display.c
 *
 *  Created on: 26 Feb 2026
 *      Author: Josh
 */

#include "display.h"

volatile uint32_t samples = 0;
volatile uint8_t row = 0;

// Representing 24 LEDs, each with RGB channels.
// There are 8 rows of 3 LEDs, and we address the LEDs per row
volatile rgb_led led_matrix[8][3] = {
		{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
		{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
		{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
		{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
		{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
		{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
		{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
		{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
};

uint32_t color_lookup[3][3] = {
		{C1R, C1G, C1B},
		{C2R, C2G, C2B},
		{C3R, C3G, C3B}
};

uint32_t row_to_row_adr[8] = {
	ROWSEL1,
	ROWSEL2,
	ROWSEL3,
	ROWSEL4,
	ROWSEL5,
	ROWSEL6,
	ROWSEL7,
	ROWSEL8
};

void display_clock_store() {
	GPIO_PinWrite(GPIO1, DISPLAY_CLOCK, 1);
	//SDK_DelayAtLeastUs(1, 150e6);
	GPIO_PinWrite(GPIO1, DISPLAY_CLOCK, 0);
	//SDK_DelayAtLeastUs(1, 150e6);
}

void display_clock_output() {
	GPIO_PinWrite(GPIO1, DISPLAY_R_CLOCK, 1);
	//SDK_DelayAtLeastUs(1, 150e6);
	GPIO_PinWrite(GPIO1, DISPLAY_R_CLOCK, 0);
	//SDK_DelayAtLeastUs(1, 150e6);
}

void display_enable() {
	GPIO_PinWrite(GPIO1, DISPLAY_OEN, 0);
}

void display_write_byte(uint8_t byte) {
	GPIO_PinWrite(GPIO1, DISPLAY_CLOCK, 0);
	GPIO_PinWrite(GPIO1, DISPLAY_R_CLOCK, 0);

	for (int i = 7; i >= 0; i--) {
		if (byte & (1 << i)) {
			GPIO_PinWrite(GPIO1, DISPLAY_DATA, 1);
		} else {
			GPIO_PinWrite(GPIO1, DISPLAY_DATA, 0);
		}
		display_clock_store();
	}
	
	display_clock_output();
}

void display_write_bytes(uint8_t *data, uint16_t len) {
	GPIO_PinWrite(GPIO1, DISPLAY_CLOCK, 0);
	GPIO_PinWrite(GPIO1, DISPLAY_R_CLOCK, 0);

	for (uint16_t i = 0; i < len; i++) {
		for (int j = 0; j < 8; j++) {
			if ((data[i]) & (1 << j)) {
				GPIO_PinWrite(GPIO1, DISPLAY_DATA, 1);
			} else {
				GPIO_PinWrite(GPIO1, DISPLAY_DATA, 0);
			}
			display_clock_store();
		}
	}
	
	display_clock_output();
}

void display_write_int(uint32_t data) {
	uint8_t out[3];
	out[0] = data & 0xff;
	out[1] = (data >> 8) & 0xff;
	out[2] = (data >> 16) & 0xff;

	display_write_bytes(out, 3);
}

void display_set_led(uint32_t row, uint32_t col, rgb_led value) {
	led_matrix[row][col].r = value.r;
	led_matrix[row][col].g = value.g;
	led_matrix[row][col].b = value.b;
}

void display_reset() {
	memset(led_matrix, 0, sizeof(rgb_led) * 24);
}

void display_callback(uint32_t flags) {
	if (samples >= DISPLAY_OVERSAMPLE) {
			samples = 0;
			row++;
	}

	if (row >= 8) {
		row = 0;
	}

	uint32_t data = DISPLAY_REST;
	// Disable the LEDs before we change colors
	// Prevents ghosting
	display_write_int(data);

	rgb_led *led_row = led_matrix[row];
	// Turn on row
	data |= row_to_row_adr[row];
	// Grab LEDs and decide which colors should be on
	data &= ~((led_row[0].r > samples) ? C1R : 0);
	data &= ~((led_row[0].g > samples) ? C1G : 0);
	data &= ~((led_row[0].b > samples) ? C1B : 0);

	data &= ~((led_row[1].r > samples) ? C2R : 0);
	data &= ~((led_row[1].g > samples) ? C2G : 0);
	data &= ~((led_row[1].b > samples) ? C2B : 0);

	data &= ~((led_row[2].r > samples) ? C3R : 0);
	data &= ~((led_row[2].g > samples) ? C3G : 0);
	data &= ~((led_row[2].b > samples) ? C3B : 0);

	display_write_int(data);

	samples++;
}
