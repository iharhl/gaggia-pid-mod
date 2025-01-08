#ifndef PID_H
#define PID_H

#include <limits>


class PIDController {
public:
    PIDController(float kp, float ki, float kd);
    ~PIDController() = default;
private:
    float m_Kp, m_Ki, m_Kd;
    double m_previousTime;
    float m_previousOutput = 0, m_previousError = 0, m_integral = 0;
    bool m_antiWindupEnabled = false;
    float m_minLimit = std::numeric_limits<float>::min(),
          m_maxLimit = std::numeric_limits<float>::max();
public:
    float compute(float setpoint, float measurement);
    void enableAntiWindup(float min, float max);
    void setOutputLimits(float min, float max);
private:
    float clipOutput(float output) const;
};


#endif //PID_H
