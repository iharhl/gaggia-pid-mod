#include "assert.h"

#include <cstdio> // todo: remove


ErrorHandler::ErrorHandler(PWMDriver* pwm, DisplayManager* gui) : m_pwm(pwm), m_gui(gui) {
}

void ErrorHandler::myAssert(const bool expression, const error_context_e context,
                            const uint8_t code, const severity_level severity) {
    // Figure out the status code
    const int status_code = find_bit_high(code);
    if (!expression) {
        // Return if failed on the same condition
        if (m_status.first == context and m_status.second == status_code)
            return;
        // Set current status member
        m_status.first = context;
        m_status.second = status_code;
        // Disable PWM if severity of the failed assert is worse than WARNING
        if (severity > WARNING) { m_pwm->setMode(false); }
        // Update the status on the display
        printf("SET context: %d, code: %d\n", context, status_code);
        m_gui->updateStatus(context, status_code);
        // Brick MCU if the severity is CRITICAL
        if (severity == CRITICAL) { } // todo: implement
    } else {
        // Reset current status if the assert passes on the same condition
        if (m_status.first == context and m_status.second == status_code) {
            printf("RESET context: %d, code: %d\n", context, status_code);
            m_status.first = CONTEXT_NONE;
            m_status.second = 0;
            m_gui->resetStatus();
        }
    }
}