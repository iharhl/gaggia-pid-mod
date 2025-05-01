#include "led.h"

#include <hardware/gpio.h>
#include <pico/time.h>

// Pico W devices use a GPIO on the WIFI chip for the LED, so when
// building for Pico W, CYW43_WL_GPIO_LED_PIN should be defined


LED::LED() {
    init();
}

void LED::init() {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, false);
}

void LED::turnOn() {
    gpio_put(PICO_DEFAULT_LED_PIN, true);
}

void LED::turnOff() {
    gpio_put(PICO_DEFAULT_LED_PIN, false);
}

void LED::blinkOnce(const unsigned delay) {
    gpio_put(PICO_DEFAULT_LED_PIN, true);
    sleep_ms(delay);
    gpio_put(PICO_DEFAULT_LED_PIN, false);
}

void LED::blinkForDuration(const unsigned delay, const unsigned duration) {
    const unsigned cycles = duration / delay; // calc the number of blink cycles
    for (unsigned i = 0; i < cycles; ++i) {
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms(delay);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
    }
}

void LED::blinkForCycles(const unsigned delay, const unsigned cycles) {
    for (unsigned i = 0; i < cycles; ++i) {
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms(delay);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
    }
}

void LED::blinkInf(const unsigned delay) {
    while(true) {
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms(delay);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        sleep_ms(delay);
    }
}
