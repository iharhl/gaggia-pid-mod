#ifndef PID_H
#define PID_H

#include <cstdint>


class PIDController {
public:
    PIDController(float kp, float ki, float kd);
    ~PIDController() = default;
private:
    float m_Kp, m_Ki, m_Kd;
    double m_previousTime;
    float m_previousOutput, m_previousError, m_integral;
    bool m_antiWindupEnabled;
    float m_minLimit, m_maxLimit; // by default set to min/max of float type
public:
    float compute(float setpoint, float measurement);
    void enableAntiWindup(float min, float max);
    void setOutputLimits(float min, float max);
private:
    float clipOutput(float output) const;
};


#endif //PID_H
