#ifndef ASSERT_H
#define ASSERT_H

#include "pwm.h"
#include "display_manager.h"

#include <cstdint>
#include <utility>


typedef enum severity_level {
    WARNING = 0,
    ERROR,
    CRITICAL,
} severity_level_e;

typedef enum error_context {
    CONTEXT_TEMP_SENSING = 0,
    CONTEXT_TEMP_CONTROL,
    CONTEXT_PROTOCOL,
    CONTEXT_DISPLAY,
    CONTEXT_NONE = 0xFF,
} error_context_e;


// Find which bit is set to ON to determine the status code
static inline unsigned find_bit_high(const uint8_t value) {
    for (int i = 0; i < 8; ++i) {
        if (value & (1 << i)) { return i; }
    }
    return 0xFF; // todo: implement handling
}


class ErrorHandler {
public:
    ErrorHandler(PWMDriver* pwm, DisplayManager* gui);
    ~ErrorHandler() = default;

    void myAssert(bool expression, error_context_e context, uint8_t code,
        severity_level severity);

private:
    PWMDriver* m_pwm;
    DisplayManager* m_gui;

    std::pair<error_context, uint8_t> m_status = std::make_pair(CONTEXT_NONE, 0);

};


#endif //ASSERT_H
