//
// Created by Ihar Hlukhau on 12/10/2024.
//

#ifndef PID_H
#define PID_H


class PIDController {
public:
    PIDController(float kp, float ki, float kd, float dt);
    ~PIDController() = default;
private:
    float m_Kp, m_Ki, m_Kd;
    float m_dT; // sampling time
    float m_previousOutput, m_previousError, m_integral;
    bool m_antiWindupEnabled;
    float m_minLimit, m_maxLimit; // not set by default
public:
    float compute(float setpoint, float measurement);
    void enableAntiWindup(float min, float max);
    void setOutputLimits(float min, float max);
private:
    float clipOutput(float output) const;
};


#endif //PID_H
