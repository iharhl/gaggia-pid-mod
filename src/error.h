#ifndef ERROR_H
#define ERROR_H

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

    void verify(bool expression, error_context_e context, uint8_t code,
        severity_level severity);

private:
    PWMDriver* m_pwm;
    DisplayManager* m_gui;

    error_context_e m_context = CONTEXT_NONE;
    uint8_t m_code = 0xFF;

};


#endif //ERROR_H
