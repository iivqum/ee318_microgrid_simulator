/*
 * main.c
 *
 *  Created on: 1 May 2026
 *      Author: Josh
 */


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
#include "switch.h"
#include "mesh.h"
#include <math.h>
/* TODO: insert other include files here. */

/* TODO: insert other definitions and declarations here. */

/*
 * @brief   Application entry point.
 */


void match(uint32_t flags) {
	// Main display refresh
	//display_callback(flags);
	CTIMER_ClearStatusFlags(CTIMER0, kCTIMER_Match0Flag);
}

void match2(uint32_t flags) {
	// Runs at 60 Hz.
	CTIMER_ClearStatusFlags(CTIMER1, kCTIMER_Match0Flag);
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

    mesh_t system;

    mesh_init(&system);

    system.points[0].what = mesh_point_type_generator;
    system.points[0].is_closed = true;

    system.points[3].is_closed = true;
    system.points[3].what = mesh_point_type_load;

    /*
    The game will start when the user flips one of the switches marked with
    a green light.

	The program will select where generation and loads are.
	A timer also begins that lasts a minute.

	The loads will appear blue and will flash indicating that the load
	isn't connected to anything.

    In this minute, the user must plan which switches to close to provide power
    to the loads located at various points.

	After the minute passes, user control will be removed and the solver will
	calculate how power flows through the network. Loads will now appear red
	with their brightness indicating how much power they receive.

	The 7 segment displays will show the different parameters given by the solver.

	The user can start a new round by flipping any of the switches.
    */

    bool success = mesh_solve(&system);

    while (1) {

    }

    /*
    switch_fetch_states();

    bool has_switch_changed;
    rgb_led led = {0, 0, 0};

    mesh_point_t *point;

    while (1) {
    	has_switch_changed = switch_fetch_states();

    	if (has_switch_changed) {
        	for (int i = 0; i < 8; i++) {
        		for (int j = 0; j < 3; j++) {
        			led.r = 0;
        			led.g = 0;
        			led.b = 0;

        			point = mesh_get_point(&system, i, 2 - j);

					if (point->what == mesh_point_type_generator) {
						led.g = 16;
					} else if (point->what == mesh_point_type_load) {
						led.b = 16;
					} else if (switch_get_state(i, j)) {
        				led.r = 31;
        			}

        			display_set_led(i, 2 - j, &led);
        		}
        	}
    	}
    }
	*/
    return 0;
}
