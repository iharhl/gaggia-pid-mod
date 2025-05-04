#ifndef PWM_H
#define PWM_H

#include <cstdint>

/*
 * The class does not actually use hardware PWM capabilities
 * as the frequency of the PWM signal is extremely low.
 *
 * Check README of the repo to find out more.
*/


class PWMDriver {
public:
    explicit PWMDriver(unsigned pin, unsigned period);
    ~PWMDriver() = default;

    void setMode(bool enabled);
    void drivePin(float pwm_duty_cycle);

private:
    unsigned m_Pin;
    uint64_t m_Period;
    bool m_Enabled = false;
    uint64_t m_PWMCycleStartTime = 0;
};



#endif //PWM_H
