#ifndef PWM_H
#define PWM_H

#include <cstdint>


// The class does not actually use hardware PWM capabilities
// as the frequency of the PWM signal is extremely low.
//
// Check README of the repo to find out more.
//


class PWMDriver {
public:
    PWMDriver(unsigned pin, unsigned period);
    ~PWMDriver() = default;

    void setMode(bool enabled);
    void drivePin(float pwm_duty_cycle);

private:
    unsigned m_Pin, m_Period;
    bool m_Enabled;
    uint64_t m_PWMCycleStartTime;
};



#endif //PWM_H
