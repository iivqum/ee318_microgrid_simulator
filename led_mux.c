/*
 * Copyright 2016-2026 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file    led_mux.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "display.h"
/* TODO: insert other include files here. */

/* TODO: insert other definitions and declarations here. */

/*
 * @brief   Application entry point.
 */

uint32_t count = 0;
uint32_t row_led = 0;
uint32_t col_led = 0;
rgb_led led = {31, 0, 0};

void match(uint32_t flags) {
	display_callback(flags);
	CTIMER_ClearStatusFlags(CTIMER0, kCTIMER_Match0Flag);
}

void match2(uint32_t flags) {
	col_led++;
	display_reset();

	if (col_led > 3) {
		row_led++;
		col_led = 0;
	}

	if (row_led > 8) {
		row_led = 0;
		uint8_t temp = led.b;
		led.b = led.g;
		led.g = led.r;
		led.r = temp;
	}

	display_set_led(row_led, col_led, led);

	CTIMER_ClearStatusFlags(CTIMER1, kCTIMER_Match0Flag);
}

void delay_match2() {
	count = 0;
	while (count <= 1000) {}
}

int main(void) {

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif

    display_enable();

    while (1) {
    }

    return 0;
}
