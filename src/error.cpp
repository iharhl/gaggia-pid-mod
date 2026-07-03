#include "error.h"


ErrorHandler::ErrorHandler(PWMDriver* pwm, DisplayManager* gui) :
    m_pwm(pwm),
    m_gui(gui) {
}

void ErrorHandler::verify(const bool expression, const uint8_t context,
                            const uint8_t code) {
    if (!expression) {
        // Update current context and code if higher prio
        if (context < m_context) {
            m_context = context;
            m_code = code;
        }
        // Update current code if same context but higher prio code
        else if (context == m_context and code < m_code) {
            m_code = code;
        }
    } else {
        // Clear the error if verification passed
        if (context == m_context and code == m_code) {
            m_context = ERROR_CONTEXT_NONE;
            m_code = ERROR_CODE_NONE;
        }
    }
}

void ErrorHandler::act() {
    // Disable PWM if error reported (only if not related to low/high temp reading)
    if (m_context != ERROR_CONTEXT_TEMPHI and m_context != ERROR_CONTEXT_TEMPLO and
            m_context != ERROR_CONTEXT_NONE)
        m_pwm->setMode(false);
    // Update status on display
    if (m_context != ERROR_CONTEXT_NONE and m_code != ERROR_CODE_NONE) {
        m_gui->updateStatus(m_context, m_code);
    } else {
        // Reset status on display if error is cleared. Enable PWM.
        m_gui->resetStatus();
        m_pwm->setMode(true);
    }
}

std::pair<uint8_t, uint8_t> ErrorHandler::get() const {
    return {m_context, m_code};
}
