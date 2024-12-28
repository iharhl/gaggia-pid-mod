#include "led.h"

// Pico W devices use a GPIO on the WIFI chip for the LED,
// so when building for Pico W, CYW43_WL_GPIO_LED_PIN will be defined
#include "pico/cyw43_arch.h"


bool LED::init() {
    if (cyw43_arch_init()) {
        // printf("Wi-Fi init failed");
        return false;
    }
    return true;
}

void LED::turnOn() {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
}

void LED::turnOff() {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
}

void LED::blinkOnce(const unsigned delay) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
    sleep_ms(delay);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
}

void LED::blinkForDuration(const unsigned delay, const unsigned duration) {
    const unsigned cycles = duration / delay; // calc the number of blink cycles
    for (int i = 0; i < cycles; ++i) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
        sleep_ms(delay);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
    }
}

void LED::blinkForCycles(const unsigned delay, const unsigned cycles) {
    for (int i = 0; i < cycles; ++i) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
        sleep_ms(delay);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
    }
}