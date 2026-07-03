#include "led.h"

#include <hardware/gpio.h>
#include <pico/time.h>

#ifdef WAVESHARE_RP2040_ZERO

#include <hardware/pio.h>
#include <hardware/clocks.h>

// WS2812 timing constants
#define WS2812_T1 3
#define WS2812_T2 3
#define WS2812_T3 4

// PIO program: 4 instructions driving WS2812 protocol on a single pin
// Bit encoding: T0H ~0.35us, T0L ~0.80us, T1H ~0.70us, T1L ~0.60us
// At sysclk 125 MHz with divider ~3.9: each cycle ~31.25ns
static const uint16_t ws2812_instructions[] = {
    0x6321,  //  0: out x, 1    side 0 [3]
    0x1223,  //  1: jmp !x, 3   side 1 [2]
    0x1200,  //  2: jmp 0       side 1 [2]
    0xA242,  //  3: nop         side 0 [2]
};

static const pio_program_t ws2812_program = {
    ws2812_instructions,  // instructions
    4,                    // length
    -1,                   // origin
    0,                    // pio_version
};

static PIO s_pio = nullptr;
static uint s_sm = 0;

static inline pio_sm_config ws2812_program_get_default_config(const uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + 0, offset + 3);
    sm_config_set_sideset(&c, 1, false, false);
    return c;
}

static void ws2812_init(const uint pin, const float freq) {
    s_pio = pio0;
    s_sm = pio_claim_unused_sm(s_pio, true);
    const uint offset = pio_add_program(s_pio, &ws2812_program);
    pio_gpio_init(s_pio, pin);
    pio_sm_set_consecutive_pindirs(s_pio, s_sm, pin, 1, true);
    pio_sm_config c = ws2812_program_get_default_config(offset);
    sm_config_set_sideset_pins(&c, pin);
    sm_config_set_out_shift(&c, false, true, 24);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
    const int cycles_per_bit = WS2812_T1 + WS2812_T2 + WS2812_T3;
    const float div = clock_get_hz(clk_sys) / (freq * cycles_per_bit);
    sm_config_set_clkdiv(&c, div);
    pio_sm_init(s_pio, s_sm, offset, &c);
    pio_sm_set_enabled(s_pio, s_sm, true);
}

static void ws2812_put_rgb(const uint8_t r, const uint8_t g, const uint8_t b) {
    // GRB format: G in bits 31:24, R in bits 23:16, B in bits 15:8
    // PIO shifts out 24 MSB-first bits then auto-pushes
    pio_sm_put_blocking(s_pio, s_sm, (static_cast<uint32_t>(g) << 24) |
                                     (static_cast<uint32_t>(r) << 16) |
                                     (static_cast<uint32_t>(b) << 8));
}

#endif //WAVESHARE_RP2040_ZERO


LED::LED() {
    init();
}

void LED::init() {
#ifdef WAVESHARE_RP2040_ZERO
    ws2812_init(PICO_DEFAULT_WS2812_PIN, 800000);
#else
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, false);
#endif
}

void LED::turnOn() {
#ifdef WAVESHARE_RP2040_ZERO
    ws2812_put_rgb(0, 255, 0);   // green
#else
    gpio_put(PICO_DEFAULT_LED_PIN, true);
#endif
}

void LED::turnOff() {
#ifdef WAVESHARE_RP2040_ZERO
    ws2812_put_rgb(0, 0, 0);     // off
#else
    gpio_put(PICO_DEFAULT_LED_PIN, false);
#endif
}

void LED::blinkOnce(const unsigned delay) {
#ifdef WAVESHARE_RP2040_ZERO
    ws2812_put_rgb(0, 0, 0);
    sleep_ms(delay);
    ws2812_put_rgb(0, 255, 0);
    sleep_ms(delay);
    ws2812_put_rgb(0, 0, 0);
#else
    gpio_put(PICO_DEFAULT_LED_PIN, false);
    sleep_ms(delay);
    gpio_put(PICO_DEFAULT_LED_PIN, true);
    sleep_ms(delay);
    gpio_put(PICO_DEFAULT_LED_PIN, false);
#endif
}

void LED::blinkForDuration(const unsigned delay, const unsigned duration) {
    if (delay > duration or delay == 0)
        return;
    const unsigned cycles = duration / (delay * 2);
    for (unsigned i = 0; i < cycles; ++i) {
#ifdef WAVESHARE_RP2040_ZERO
        ws2812_put_rgb(0, 255, 0);
        sleep_ms(delay);
        ws2812_put_rgb(0, 0, 0);
        sleep_ms(delay);
#else
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms(delay);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        sleep_ms(delay);
#endif
    }
}

void LED::blinkForCycles(const unsigned delay, const unsigned cycles) {
    for (unsigned i = 0; i < cycles; ++i) {
#ifdef WAVESHARE_RP2040_ZERO
        ws2812_put_rgb(0, 255, 0);
        sleep_ms(delay);
        ws2812_put_rgb(0, 0, 0);
        sleep_ms(delay);
#else
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms(delay);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        sleep_ms(delay);
#endif
    }
}

void LED::blinkInf(const unsigned delay) {
    while(true) {
#ifdef WAVESHARE_RP2040_ZERO
        ws2812_put_rgb(255, 0, 0);
        sleep_ms(delay);
        ws2812_put_rgb(0, 0, 0);
        sleep_ms(delay);
#else
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms(delay);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        sleep_ms(delay);
#endif
    }
}
