#include "pwm.h"
#include "time.h"

#include <hardware/gpio.h>


PWMDriver::PWMDriver(const unsigned pin, const unsigned period) :
                    m_Pin(pin),
                    m_Period(period),
                    m_Enabled(false),
                    m_PWMCycleStartTime(0)
{
    // Set pin as gpio output
    gpio_init(m_Pin);
    gpio_set_dir(m_Pin, GPIO_OUT);
    gpio_put(m_Pin, false);
}

void PWMDriver::setMode(const bool enabled) {
    m_Enabled = enabled;
    if (m_Enabled) { m_PWMCycleStartTime = Clock::now_ms(); }
}

void PWMDriver::drivePin(const float pwm_duty_cycle) {
    if (!m_Enabled)
        return;
    // Update PWM cycle start time every period
    if (m_Period < Clock::now_ms() - m_PWMCycleStartTime)
      m_PWMCycleStartTime += m_Period;
    // Compute how many ms PWM should stay active in the current cycle and
    // compare that to the time that have already passed
    const float pwm_active_ms = pwm_duty_cycle * (m_Period / 100.0);
    if (pwm_active_ms > 100 and pwm_active_ms > Clock::now_ms() - m_PWMCycleStartTime)
        gpio_put(m_Pin, true);
    else
        gpio_put(m_Pin, false);
}


