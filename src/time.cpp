#include "time.h"
#include <hardware/timer.h>


uint64_t Timer::now_ms() {
    return time_us_64() / 1000;
}

uint64_t Timer::now_sec() {
    return time_us_64() / (1000 * 1000);
}

uint64_t Timer::now_min() {
    return time_us_64() / (1000 * 1000 * 60);
}
