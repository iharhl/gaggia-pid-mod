#include "pico/stdlib.h"
#include "pid.h"

int main() {
    constexpr uint LED_PIN = 25;

    PIDController mypid(1,0,0,1);
    float res = mypid.compute(10, 30);


    gpio_init (LED_PIN);
    gpio_set_dir (LED_PIN, GPIO_OUT);
    while (true) {
        gpio_put (LED_PIN, 1);
        sleep_ms (int(res));
        gpio_put (LED_PIN, 0);
        sleep_ms (250);
    }
}