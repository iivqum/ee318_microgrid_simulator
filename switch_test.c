/*
 * Copyright 2016-2026 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file    switch_test.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
/* TODO: insert other include files here. */

/* TODO: insert other definitions and declarations here. */

// Port 1
#define CLOCK 14
#define DATA 16
#define OEN  18
#define R_CLOCK 22
#define MUX_OUT 6

/*
16 bit address structure

MSB	0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 LSB
	C1 C2 C3 R1 R2 R3 R4 R5 R6 R7 R8 A1 A2 X  X  X

			   Rows			  Cols   Address   NC
*/
#define C1 (uint16_t)(1 << 15)
#define C2 (uint16_t)(1 << 14)
#define C3 (uint16_t)(1 << 13)

#define R1 (uint16_t)(1 << 12)
#define R2 (uint16_t)(1 << 11)
#define R3 (uint16_t)(1 << 10)
#define R4 (uint16_t)(1 << 9)
#define R5 (uint16_t)(1 << 8)
#define R6 (uint16_t)(1 << 7)
#define R7 (uint16_t)(1 << 6)
#define R8 (uint32_t)(1 << 5)

#define A1 (uint16_t)(1 << 4)
#define A2 (uint16_t)(1 << 3)

uint32_t column_lookup[3] = {C1, C2, C3};
uint32_t address_lookup[3] = {0, A1, A2};
uint32_t row_lookup[8] = {R1, R2, R3, R4, R5, R6, R7, R8};

#define ROW_IDLE (uint16_t)(0xff << 5);

#define CLOCK_SPEED 150e6

void sr_clock_store() {
	GPIO_PinWrite(GPIO1, CLOCK, 1);
	//SDK_DelayAtLeastUs(1, 150e6);
	GPIO_PinWrite(GPIO1, CLOCK, 0);
	//SDK_DelayAtLeastUs(1, 150e6);
}

void sr_clock_output() {
	GPIO_PinWrite(GPIO1, R_CLOCK, 1);
	//SDK_DelayAtLeastUs(1, 150e6);
	GPIO_PinWrite(GPIO1, R_CLOCK, 0);
	//SDK_DelayAtLeastUs(1, 150e6);
}

void sr_enable() {
	GPIO_PinWrite(GPIO1, OEN, 0);
}

void sr_disable() {
	GPIO_PinWrite(GPIO1, OEN, 1);
}

void sr_write_byte(uint8_t byte) {
	GPIO_PinWrite(GPIO1, CLOCK, 0);
	GPIO_PinWrite(GPIO1, R_CLOCK, 0);

	sr_disable();

	for (int i = 0; i < 8; i++) {
		if (byte & (1 << i)) {
			GPIO_PinWrite(GPIO1, DATA, 1);
		} else {
			GPIO_PinWrite(GPIO1, DATA, 0);
		}
		sr_clock_store();
	}

	sr_clock_output();
	sr_enable();
}

void sr_write_bytes(uint8_t *data, uint16_t len) {
	GPIO_PinWrite(GPIO1, CLOCK, 0);
	GPIO_PinWrite(GPIO1, R_CLOCK, 0);

	sr_disable();

	for (uint16_t i = 0; i < len; i++) {
		for (int j = 0; j < 8; j++) {
			if ((data[i]) & (1 << j)) {
				GPIO_PinWrite(GPIO1, DATA, 1);
			} else {
				GPIO_PinWrite(GPIO1, DATA, 0);
			}
			sr_clock_store();
		}
	}

	sr_clock_output();
	sr_enable();
}

void sr_write_16(uint16_t data) {
	uint8_t out[2];
	out[0] = data & 0xff;
	out[1] = (data >> 8) & 0xff;

	sr_write_bytes(out, 2);
}
uint32_t mux_read() {
	return GPIO_PinRead(GPIO1, MUX_OUT);
}

/*
 * @brief   Application entry point.
 */
int main(void) {

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif



    /*
	16 bit address structure

	MSB	0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 LSB
		C1 C2 C3 R1 R2 R3 R4 R5 R6 R7 R8 A1 A2 X  X  X

    		       Rows			  Cols   Address   NC
    */

    uint16_t data;
    uint32_t state;

    PRINTF("-------------------------------------\n\r");

    for (int row = 0; row < 8; row++) {

    	for (int col = 0; col < 3; col++) {
    		data = ROW_IDLE;
    		data |= column_lookup[col];
    		data |= address_lookup[col];
    		data &= ~row_lookup[row];

    		sr_write_16(data);
    		// Wait for switches to settle
    		SDK_DelayAtLeastUs(10e3, 150e6);

    		state = mux_read();

    		PRINTF("R%dC%d : %s\t", row + 1, col + 1, state ? "OFF" : "ON");
    	}

    	PRINTF("\n\r");
    }

    return 0;
}
