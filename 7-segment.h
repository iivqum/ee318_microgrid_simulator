#ifndef TM1637_H_
#define TM1637_H_

#include <stdint.h>
#include <stdbool.h>
#include "fsl_gpio.h"

/* Configuration Parameters */
#define TM_COUNT 5U

/* Pin mapping definition required for external implementation */
typedef struct {
    GPIO_Type *clk_gpio;
    uint32_t clk_pin;
    GPIO_Type *dio_gpio[TM_COUNT];
    uint32_t dio_pin[TM_COUNT];
} tm1637_config_t;

/* Public Interfaces */
/**
 * @brief Initialize TM1637 GPIOs with a specific configuration
 * @param config Pointer to the configuration struct
 */
void TM1637_Init(const tm1637_config_t *config);

/**
 * @brief Clear a specific display
 */
bool TM1637_Clear(uint8_t index, uint8_t brightness);

/**
 * @brief Display a 4-digit decimal number
 */
bool TM1637_DisplayNumber(uint8_t index, uint16_t num, uint8_t brightness);

/**
 * @brief Send raw segment data to the display
 */
bool TM1637_DisplayRaw(uint8_t index, const uint8_t data[4], uint8_t brightness);

#endif /* TM1637_H_ */
