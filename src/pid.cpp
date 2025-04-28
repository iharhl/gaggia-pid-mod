#include "pid.h"
#include "clock.h"


PIDController::PIDController(const float kp, const float ki, const float kd) :
    m_Kp(kp),
    m_Ki(ki),
    m_Kd(kd),
    m_previousTime(Clock::now_ms()) {
}

float PIDController::compute(const float setpoint, const float measurement) {
    // Calculate error
    const float error = setpoint - measurement;

    // Calculate dT (elapsed time since last iteration) in seconds
    // and update the prev timestamp
    const float dT = static_cast<float>(Clock::now_ms() - m_previousTime) / 1000;
    m_previousTime = Clock::now_ms();

    // Calculate integral
    //
    // If anti-windup is enabled and output is saturated, and it has same sign as error
    // (accumulates in the same direction) -> integrator should stop accumulating
    const bool is_output_saturated = m_previousOutput >= m_maxLimit or m_previousOutput <= m_minLimit;
    const bool is_output_and_error_same_sign = (m_integral > 0) ^ (m_previousOutput <= 0);
    if (!(m_antiWindupEnabled and is_output_saturated and is_output_and_error_same_sign))
        m_integral += error * dT;

    // Calculate derivative
    const float derivative = (error - m_previousError) / dT;

    // Calculate and clip output
    float output = m_Kp * error + m_Ki * m_integral + m_Kd * derivative;
    output = getClippedOutput(output);

    m_previousError = error;
    m_previousOutput = output;
    return output;
}

/*
 * Enable integral anti-windup using clamping technique
 * min : min limit of PID output
 * max : max limit of PID output
 */
void PIDController::enableAntiWindup(const float min, const float max) {
    m_antiWindupEnabled = true;
    m_minLimit = min;
    m_maxLimit = max;
}

void PIDController::setOutputLimits(const float min, const float max) {
    m_minLimit = min;
    m_maxLimit = max;
}

float PIDController::getClippedOutput(const float output) const {
    if (output > m_maxLimit)
        return m_maxLimit;
    if (output < m_minLimit)
        return m_minLimit;
    return output;
}
