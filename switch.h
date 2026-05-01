/*
 * switch.h
 *
 *  Created on: 1 May 2026
 *      Author: Josh
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifndef SWITCH_H_
#define SWITCH_H_

#define SWITCH_GPIO_PORT 1

#define SWITCH_CLOCK 19
#define SWITCH_DATA 17
#define SWITCH_OEN  15
#define SWITCH_R_CLOCK 5
#define SWITCH_SENSE 7

/*
16 bit address structure

MSB	0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 LSB
	C1 C2 C3 R1 R2 R3 R4 R5 R6 R7 R8 A1 A2 X  X  X

			   Rows			  Cols   Address   NC
*/
#define SWITCH_C1 (uint32_t)(1 << 15)
#define SWITCH_C2 (uint32_t)(1 << 14)
#define SWITCH_C3 (uint32_t)(1 << 13)

#define SWITCH_R1 (uint32_t)(1 << 12)
#define SWITCH_R2 (uint32_t)(1 << 11)
#define SWITCH_R3 (uint32_t)(1 << 10)
#define SWITCH_R4 (uint32_t)(1 << 9)
#define SWITCH_R5 (uint32_t)(1 << 8)
#define SWITCH_R6 (uint32_t)(1 << 7)
#define SWITCH_R7 (uint32_t)(1 << 6)
#define SWITCH_R8 (uint32_t)(1 << 5)

#define SWITCH_A1 (uint32_t)(1 << 4)
#define SWITCH_A2 (uint32_t)(1 << 3)

#define SWITCH_ROW_IDLE (uint32_t)(0xff << 5);

void switch_disable();
void switch_enable();
void switch_write_byte(uint8_t byte);
void switch_write_bytes(uint8_t *data, uint16_t length);
void switch_clock_store();
void switch_clock_output();
void switch_write_16(uint16_t data);
bool switch_get_state(uint8_t row, uint8_t col);
void switch_fetch_states();

#endif /* SWITCH_H_ */
