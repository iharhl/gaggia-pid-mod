#ifndef ERROR_H
#define ERROR_H

#include "pwm.h"
#include "display_manager.h"

#include <cstdint>


typedef enum error_context {
    ERROR_CONTEXT_TEMPSENS = 0,
    ERROR_CONTEXT_TEMPHI,
    ERROR_CONTEXT_TEMPLO,
    ERROR_CONTEXT_COMM,
    ERROR_CONTEXT_DISPLAY,
    ERROR_CONTEXT_NONE = 0xFF,
} error_context_e;

typedef enum error_code {
    ERROR_CODE_0 = 0,
    ERROR_CODE_1,
    ERROR_CODE_2,
    ERROR_CODE_3,
    ERROR_CODE_4,
    ERROR_CODE_5,
    ERROR_CODE_6,
    ERROR_CODE_7,
    ERROR_CODE_8,
    ERROR_CODE_9,
    ERROR_CODE_I, // trick to show HI status, where I is in place of digit
    ERROR_CODE_NONE = 0xFF,
} error_code_e;


class ErrorHandler {
public:
    ErrorHandler(PWMDriver* pwm, DisplayManager* gui);
    ~ErrorHandler() = default;

    void verify(bool expression, uint8_t context, uint8_t code);

private:
    PWMDriver* m_pwm;
    DisplayManager* m_gui;

    uint8_t m_context = ERROR_CONTEXT_NONE;
    uint8_t m_code = ERROR_CODE_NONE;

};


#endif //ERROR_H
