#include "time.h"
#include <hardware/timer.h>


uint64_t Clock::now_ms() {
    return time_us_64() / 1000;
}

uint64_t Clock::now_sec() {
    return time_us_64() / (1000 * 1000);
}

uint64_t Clock::now_min() {
    return time_us_64() / (1000 * 1000 * 60);
}
