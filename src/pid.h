#ifndef PID_H
#define PID_H

#include <cstdint>


class PIDController {
public:
    explicit PIDController(float kp, float ki, float kd);
    ~PIDController() = default;
private:
    float m_Kp, m_Ki, m_Kd;
    uint64_t m_previousTime;
    float m_previousOutput = 0, m_previousError = 0, m_integral = 0;
    bool m_antiWindupEnabled = false;
    float m_minLimit = 0, m_maxLimit = 100;
public:
    float compute(float setpoint, float measurement);
    void enableAntiWindup(float min, float max);
    void setOutputLimits(float min, float max);
private:
    [[nodiscard]] float getClippedOutput(float output) const;
};


#endif //PID_H
