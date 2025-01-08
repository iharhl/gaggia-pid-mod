#ifndef ERROR_H
#define ERROR_H

#include "pwm.h"
#include "display_manager.h"

#include <cstdint>


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

typedef enum error_code {
    CODE_0 = 0b00000001,
    CODE_1 = 0b00000010,
    CODE_2 = 0b00000100,
    CODE_3 = 0b00001000,
    CODE_4 = 0b00010000,
    CODE_5 = 0b00100000,
    CODE_6 = 0b01000000,
    CODE_7 = 0b10000000,
    CODE_NONE = 0xFF,
} error_code_e;


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

    void verify(bool expression, uint8_t context, uint8_t code,
        severity_level severity);

private:
    PWMDriver* m_pwm;
    DisplayManager* m_gui;

    uint8_t m_context = CONTEXT_NONE;
    uint8_t m_code = CODE_NONE;

};


#endif //ERROR_H
