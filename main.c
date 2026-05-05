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
#include "7-segment.h"
#include "mesh.h"
#include <math.h>
/* TODO: insert other include files here. */

/* TODO: insert other definitions and declarations here. */

/*
 * @brief   Application entry point.
 */

#define ROUND_TIME 10

// 7 segment display configuration
const tm1637_config_t segment_display_config = {
    .clk_gpio = GPIO4,
    .clk_pin  = 6,
    .dio_gpio = {GPIO4, GPIO1, GPIO1, GPIO1, GPIO1},
    .dio_pin  = {1, 0, 0, 0, 0}
};

typedef enum game_state {
	// Waiting for user input.
	game_state_waiting,
	// Planning phase where user decides on grid connections
	game_state_planning,
	// Solution phase. User can see how they did
	game_state_solution
} game_state_t;

game_state_t state = game_state_waiting;
mesh_t microgrid;
uint8_t round_timer = ROUND_TIME;

void reset_displays() {
	TM1637_DisplayNumber(0, round_timer, 5U);
}

void game_state_transition(game_state_t);

void match(uint32_t flags) {
	// Main display refresh
	display_callback(flags);
	CTIMER_ClearStatusFlags(CTIMER0, kCTIMER_Match0Flag);
}

void timer1_60hz_match(uint32_t flags) {
	// Runs at 60 Hz.
	//CTIMER_ClearStatusFlags(CTIMER1, kCTIMER_Match0Flag);
}

void timer1_1hz_match(uint32_t flags) {
	// Runs at 1 Hz.
	if (state == game_state_planning) {
		if (round_timer == 0) {
			game_state_transition(game_state_solution);
		} else {
			round_timer--;
			TM1637_DisplayNumber(0, round_timer, 5U);
		}
	}
	CTIMER_ClearStatusFlags(CTIMER1, kCTIMER_Match1Flag);
}

void game_state_transition(game_state_t new_state) {
	if (new_state == game_state_planning) {
		switch_reset_fake_states();
		round_timer = ROUND_TIME;
	}

	if (new_state == game_state_solution) {
		reset_displays();

		mesh_resolve(&microgrid);
		mesh_solve(&microgrid);

		mesh_point_t *point;
		rgb_led led = {0, 0, 0};

		if (!microgrid.solution_valid) {
			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < 3; j++) {
					point = mesh_get_point_display_mapped(&microgrid, i, j);
					led.r = 31;
					led.g = 0;
					led.b = 0;

					display_set_led(i, j, &led);
				}
			}
		} else {
			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < 3; j++) {
					point = mesh_get_point_display_mapped(&microgrid, i, j);
					led.r = 0;
					led.g = 0;
					led.b = 0;

					float v = fabsf(point->voltage);
					float gamma = 1.6;

					if (v < 0.0f) v = 0.0f;
					if (v > 1.0f) v = 1.0f;

					float corrected = powf(v, gamma);

					if (point->what == mesh_point_type_load) {
						led.b = (uint8_t)(31.0f * corrected);
					} else {
						led.r = (uint8_t)(31.0f * corrected);
						led.g = (uint8_t)(31.0f * corrected);
					}

					display_set_led(i, j, &led);
				}
			}
		}

		mesh_reset_connections(&microgrid);

		// Change the  grid configuration here
	}

	state = new_state;
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

    mesh_init(&microgrid);

    microgrid.points[0].what = mesh_point_type_generator;
    microgrid.points[0].is_closed = true;

    microgrid.points[21].what = mesh_point_type_load;
    microgrid.points[21].is_closed = true;

    microgrid.points[23].what = mesh_point_type_load;
    microgrid.points[23].is_closed = true;

	mesh_resolve(&microgrid);
	mesh_solve(&microgrid);

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
	TM1637_Init(&segment_display_config);
	reset_displays();

	display_reset();

    switch_reset_fake_states();
    switch_fetch_states();

    bool has_switch_changed;
    rgb_led led = {0, 0, 0};
    mesh_point_t *point;

    while (1) {
    	has_switch_changed = switch_fetch_states();

    	if (has_switch_changed) {
    		if (state == game_state_waiting) {
    			game_state_transition(game_state_planning);
    		}

    		if (state == game_state_solution) {
    			game_state_transition(game_state_planning);
    		}

    		// In planning mode, you can only toggle connections
    		if (state == game_state_planning) {
				for (int i = 0; i < 8; i++) {
					for (int j = 0; j < 3; j++) {
						bool switch_changed = switch_get_state(i, 2 - j, false);
						point = mesh_get_point_display_mapped(&microgrid, i, j);
						// If it changed
						if (switch_changed) {
							if (point->what == mesh_point_type_connection) {
								point->is_closed = !point->is_closed;
								// Toggle connections in planning mode
							}
						}

						led.r = 0;
						led.g = 0;
						led.b = 0;

						switch (point->what) {
							case mesh_point_type_connection:
								if (point->is_closed) {
									led.r = 1;
								}
								break;
							case mesh_point_type_load:
								led.b = 1;
								break;
							case mesh_point_type_generator:
								led.g = 1;
						}

						display_set_led(i, j, &led);
					}
				}
    		}

    		/*

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

    		*/
    	}
    }
    return 0;
}
