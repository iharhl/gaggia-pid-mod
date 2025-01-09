#ifndef TIME_H
#define TIME_H

#include <cstdint>


class Clock {
public:
    static uint64_t now_ms();
    static uint64_t now_sec();
    static uint64_t now_min();
};


#endif //TIME_H
