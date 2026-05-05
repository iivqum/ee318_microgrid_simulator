#include "7-segment.h"

/* Internal copy of configuration */
static tm1637_config_t g_tm_config;

/* Segment patterns for digits 0-9 */
static const uint8_t seg_digits[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

#define SEG_BLANK 0x00
static const gpio_pin_config_t tm_out_low_config = {kGPIO_DigitalOutput, 0U};
static const gpio_pin_config_t tm_in_config      = {kGPIO_DigitalInput, 0U};

/* Low-level hardware abstraction using the config struct */
static inline void CLK_HIGH(void) { GPIO_PinWrite(g_tm_config.clk_gpio, g_tm_config.clk_pin, 1U); }
static inline void CLK_LOW(void)  { GPIO_PinWrite(g_tm_config.clk_gpio, g_tm_config.clk_pin, 0U); }
static inline void DIO_LOW(uint8_t index) {
    GPIO_PinInit(g_tm_config.dio_gpio[index], g_tm_config.dio_pin[index], &tm_out_low_config);
}
static inline void DIO_RELEASE(uint8_t index) {
    GPIO_PinInit(g_tm_config.dio_gpio[index], g_tm_config.dio_pin[index], &tm_in_config);
}
static inline uint32_t DIO_READ(uint8_t index) {
    return GPIO_PinRead(g_tm_config.dio_gpio[index], g_tm_config.dio_pin[index]);
}

static void tm_delay(void) {
    SDK_DelayAtLeastUs(5, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
}

static void tm_start(uint8_t index) {
    for (uint8_t i = 0; i < TM_COUNT; i++) DIO_RELEASE(i);
    DIO_RELEASE(index); CLK_HIGH(); tm_delay();
    DIO_LOW(index); tm_delay();
    CLK_LOW(); tm_delay();
}

static void tm_stop(uint8_t index) {
    CLK_LOW(); DIO_LOW(index); tm_delay();
    CLK_HIGH(); tm_delay();
    DIO_RELEASE(index); tm_delay();
}

static bool tm_write_byte(uint8_t index, uint8_t data) {
    for (uint8_t i = 0; i < 8U; i++) {
        CLK_LOW(); tm_delay();
        if (data & 0x01U) DIO_RELEASE(index); else DIO_LOW(index);
        tm_delay(); CLK_HIGH(); tm_delay();
        data >>= 1;
    }
    CLK_LOW(); DIO_RELEASE(index); tm_delay();
    CLK_HIGH(); tm_delay();
    bool ack = (DIO_READ(index) == 0U);
    CLK_LOW(); tm_delay();
    return ack;
}

/* API Implementation */
void TM1637_Init(const tm1637_config_t *config) {
    /* Copy configuration to local storage */
    g_tm_config = *config;

    gpio_pin_config_t clk_cfg = {kGPIO_DigitalOutput, 1U};
    GPIO_PinInit(g_tm_config.clk_gpio, g_tm_config.clk_pin, &clk_cfg);

    for (uint8_t i = 0; i < TM_COUNT; i++) {
        DIO_RELEASE(i);
    }
}

bool TM1637_DisplayRaw(uint8_t index, const uint8_t data[4], uint8_t brightness) {
    bool ok = true;
    tm_start(index);
    ok &= tm_write_byte(index, 0x40U); // Automatic address increment
    tm_stop(index);

    tm_start(index);
    ok &= tm_write_byte(index, 0xC0U); // Start address
    for (uint8_t i = 0; i < 4U; i++) ok &= tm_write_byte(index, data[i]);
    tm_stop(index);

    tm_start(index);
    ok &= tm_write_byte(index, (uint8_t)(0x88U | (brightness & 0x07U)));
    tm_stop(index);
    return ok;
}

bool TM1637_Clear(uint8_t index, uint8_t brightness) {
    uint8_t blank[4] = {0, 0, 0, 0};
    return TM1637_DisplayRaw(index, blank, brightness);
}

bool TM1637_DisplayNumber(uint8_t index, uint16_t num, uint8_t brightness) {
    uint8_t buf[4];
    buf[3] = seg_digits[num % 10U];
    buf[2] = seg_digits[(num / 10U) % 10U];
    buf[1] = seg_digits[(num / 100U) % 10U];
    buf[0] = seg_digits[(num / 1000U) % 10U];

    if (buf[0] == seg_digits[0]) {
        buf[0] = SEG_BLANK;
        if (buf[1] == seg_digits[0]) {
            buf[1] = SEG_BLANK;
            if (buf[2] == seg_digits[0]) buf[2] = SEG_BLANK;
        }
    }
    return TM1637_DisplayRaw(index, buf, brightness);
}
