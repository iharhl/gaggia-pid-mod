#include "pwm.h"
#include <hardware/gpio.h>
#include "time.h"


PWMDriver::PWMDriver(const unsigned pin, const unsigned period) :
                    m_Period(period),
                    m_Enabled(false),
                    m_PWMCycleStartTime(0)
{
    // Set pin as gpio output
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, false);
}

void PWMDriver::setMode(const bool enabled) {
    m_Enabled = enabled;
    if (m_Enabled) { m_PWMCycleStartTime = Timer::now_ms(); }
}

bool PWMDriver::isDriven(const float pwm_duty_cycle) {
    if (!m_Enabled)
        return false;
    // Update PWM cycle start time every period
    if (m_Period < Timer::now_ms() - m_PWMCycleStartTime)
      m_PWMCycleStartTime += m_Period;
    // Compute how many ms PWM should stay active in the current cycle and
    // compare that to the time that have already passed
    const float pwm_active_ms = pwm_duty_cycle * (m_Period / 100.0);
    if (pwm_active_ms > 100 and pwm_active_ms > Timer::now_ms() - m_PWMCycleStartTime)
        return true;
    return false;
}


