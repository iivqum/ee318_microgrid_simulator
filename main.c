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
	display_callback(flags);
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

    system.points[7].is_closed = true;

    system.points[14].is_closed = true;

    system.points[21].what = mesh_point_type_load;
    system.points[21].is_closed = true;

	mesh_resolve(&system);
	mesh_solve(&system);

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
    switch_reset_fake_states();
    switch_fetch_states();

    bool has_switch_changed;
    rgb_led led = {0, 0, 0};
    mesh_point_t *point;

    while (1) {
    	has_switch_changed = switch_fetch_states();

    	if (has_switch_changed) {
    		for (int i = 0; i < 8; i++) {
    			for (int j = 0; j < 3; j++) {
    				bool state = switch_get_state(i, 2 - j, false);
    				// If it changed
    				if (state) {
						point = mesh_get_point_display_mapped(&system, i, j);
						point->is_closed = !point->is_closed;
    				}
    			}
    		}

    		mesh_resolve(&system);
    		mesh_solve(&system);

    		for (int i = 0; i < 8; i++) {
    			for (int j = 0; j < 3; j++) {
    				led.r = 0;
    				led.g = 0;
    				led.b = 0;

    				point = mesh_get_point_display_mapped(&system, i, j);

    				if (point->what == mesh_point_type_generator) {
    					led.g = 31;
    				} else {
    					led.r = (float)31 * (float)fabs(point->voltage);
    				}

    				display_set_led(i, j, &led);
    			}
    		}
    	}
    }
    return 0;
}
