#include "error.h"


ErrorHandler::ErrorHandler(PWMDriver* pwm, DisplayManager* gui) :
    m_pwm(pwm),
    m_gui(gui) {
}

void ErrorHandler::verify(const bool expression, const uint8_t context,
                            const uint8_t code) {
    if (!expression) {
        // Disable PWM (only if not related to low/high temp reading)
        if (context != ERROR_CONTEXT_TEMPHI and context != ERROR_CONTEXT_TEMPLO)
            m_pwm->setMode(false);
        // Update the status on the display
        m_gui->updateStatus(context, code);
    } else {
        m_gui->resetStatus(context, code);
    }
}
