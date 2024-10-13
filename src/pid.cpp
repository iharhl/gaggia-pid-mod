//
// Created by Ihar Hlukhau on 12/10/2024.
//

#include "pid.h"

#include <limits>


PIDController::PIDController(const float kp, const float ki, const float kd, const float dt) :
    m_Kp(kp),
    m_Ki(ki),
    m_Kd(kd),
    m_dT(dt),
    m_previousOutput(0),
    m_previousError(0),
    m_integral(0),
    m_antiWindupEnabled(false),
    m_minLimit(std::numeric_limits<float>::min()),
    m_maxLimit(std::numeric_limits<float>::max()) {}

float PIDController::compute(const float setpoint, const float measurement) {
    // Calculate error
    const float error = setpoint - measurement;

    // Calculate integral
    m_integral += error * m_dT;
    // TODO: clean up
    if (m_antiWindupEnabled) {
        if ((m_previousOutput > m_maxLimit or m_previousOutput < m_minLimit) and
           ((m_integral > 0) ^ (m_previousOutput < 0)) // check for same sign
        ) {
            m_integral = 0;  // clamp integral part
        }
    }

    // Calculate derivative
    const float derivative = (error - m_previousError) / m_dT;

    // Calculate and clip output
    float output = m_Kp * error + m_Ki * m_integral + m_Kd * derivative;
    output = this->_clipOutput(output);

    m_previousError = error;
    return output;
}

/* Enable integral anti-windup using clamping technique
* min : min limit of PID output
* max : max limit of PID output */
void PIDController::enableAntiWindup(const float min, const float max) {
    m_antiWindupEnabled = true;
    m_minLimit = min;
    m_maxLimit = max;
}

void PIDController::setOutputLimits(const float min, const float max) {
    m_minLimit = min;
    m_maxLimit = max;
}

float PIDController::_clipOutput(const float output) const {
    if (output > m_maxLimit) { return m_maxLimit; }
    if (output < m_minLimit) { return m_minLimit; }
    return output;
}
