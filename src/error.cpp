#include "error.h"

#include <cstdio> // todo: remove


ErrorHandler::ErrorHandler(PWMDriver* pwm, DisplayManager* gui) : m_pwm(pwm), m_gui(gui) {
}

void ErrorHandler::verify(const bool expression, const error_context_e context,
                            const uint8_t code, const severity_level severity) {
    // Figure out the status code number
    const int status_code = find_bit_high(code);
    if (!expression) {
        // Disable PWM if severity of the failed assert is worse than WARNING
        if (severity > WARNING) { m_pwm->setMode(false); }
        // Update the status on the display
        m_gui->updateStatus(context, status_code);
        // Brick MCU if the severity is CRITICAL
        // if (severity == CRITICAL) { ... }
    } else {
        m_gui->resetStatus(context, status_code);
    }
}
