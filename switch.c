/*
 * switch.c
 *
 *  Created on: 1 May 2026
 *      Author: Josh
 */


#include "switch.h"

uint32_t column_lookup[3] = {SWITCH_C1, SWITCH_C2, SWITCH_C3};
uint32_t address_lookup[3] = {0, SWITCH_A1, SWITCH_A2};
uint32_t row_lookup[8] = {SWITCH_R1, SWITCH_R2, SWITCH_R3,
		SWITCH_R4, SWITCH_R5, SWITCH_R6, SWITCH_R7, SWITCH_R8};

bool switch_states[SWITCH_NUM_ROWS][SWITCH_NUM_COLS] = {false};

bool switch_get_state(uint8_t row, uint8_t col) {
	return switch_states[row][col];
}

void switch_fetch_states() {
	uint16_t data;

    for (int row = 0; row < 8; row++) {
    	for (int col = 0; col < 3; col++) {
    		data = SWITCH_ROW_IDLE;
    		data |= column_lookup[col];
    		data |= address_lookup[col];
    		data &= ~row_lookup[row];

    		switch_write_16(data);
    		// Wait for switches to settle
    		SDK_DelayAtLeastUs(100, 150e6);

    		switch_states[row][col] = GPIO_PinRead(SWITCH_GPIO_PORT, SWITCH_SENSE);
    	}
    }
}

void switch_clock_store() {
	GPIO_PinWrite(SWITCH_GPIO_PORT, SWITCH_CLOCK, 1);
	//SDK_DelayAtLeastUs(1, 150e6);
	GPIO_PinWrite(SWITCH_GPIO_PORT, SWITCH_CLOCK, 0);
	//SDK_DelayAtLeastUs(1, 150e6);
}

void switch_clock_output() {
	GPIO_PinWrite(SWITCH_GPIO_PORT, SWITCH_R_CLOCK, 1);
	//SDK_DelayAtLeastUs(1, 150e6);
	GPIO_PinWrite(SWITCH_GPIO_PORT, SWITCH_R_CLOCK, 0);
	//SDK_DelayAtLeastUs(1, 150e6);
}

void switch_enable() {
	GPIO_PinWrite(SWITCH_GPIO_PORT, SWITCH_OEN, 0);
}

void switch_disable() {
	GPIO_PinWrite(SWITCH_GPIO_PORT, SWITCH_OEN, 1);
}

void switch_write_byte(uint8_t byte) {
	GPIO_PinWrite(SWITCH_GPIO_PORT, SWITCH_CLOCK, 0);
	GPIO_PinWrite(SWITCH_GPIO_PORT, SWITCH_R_CLOCK, 0);

	switch_disable();

	for (int i = 0; i < 8; i++) {
		if (byte & (1 << i)) {
			GPIO_PinWrite(SWITCH_GPIO_PORT, SWITCH_DATA, 1);
		} else {
			GPIO_PinWrite(SWITCH_GPIO_PORT, SWITCH_DATA, 0);
		}
		switch_clock_store();
	}

	switch_clock_output();
	switch_enable();
}

void switch_write_bytes(uint8_t *data, uint16_t len) {
	GPIO_PinWrite(SWITCH_GPIO_PORT, SWITCH_CLOCK, 0);
	GPIO_PinWrite(SWITCH_GPIO_PORT, SWITCH_R_CLOCK, 0);

	switch_disable();

	for (uint16_t i = 0; i < len; i++) {
		for (int j = 0; j < 8; j++) {
			if ((data[i]) & (1 << j)) {
				GPIO_PinWrite(SWITCH_GPIO_PORT, SWITCH_DATA, 1);
			} else {
				GPIO_PinWrite(SWITCH_GPIO_PORT, SWITCH_DATA, 0);
			}
			switch_clock_store();
		}
	}

	switch_clock_output();
	switch_enable();
}

void switch_write_16(uint16_t data) {
	uint8_t out[2];
	out[0] = data & 0xff;
	out[1] = (data >> 8) & 0xff;

	switch_write_bytes(out, 2);
}
