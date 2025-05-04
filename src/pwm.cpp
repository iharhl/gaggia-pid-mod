#include "pwm.h"
#include "clock.h"

#include <hardware/gpio.h>
#include <hardware/timer.h>
#include <pico/time.h>


PWMDriver::PWMDriver(const unsigned pin, const unsigned period) :
                    m_Pin(pin),
                    m_Period(period)
{
    // Set pin as gpio output
    gpio_init(m_Pin);
    gpio_set_dir(m_Pin, GPIO_OUT);
    gpio_put(m_Pin, false);
    // Enable PWM
    m_Enabled = true;
}

void PWMDriver::setMode(const bool enabled) {
    m_Enabled = enabled;
    if (!m_Enabled)
        gpio_put(m_Pin, false);
}

void PWMDriver::drivePin(const float pwm_duty_cycle) {
    if (!m_Enabled)
        return;
    // Only drive the pin at the beginning of the cycle
    if (m_Period < Clock::now_ms() - m_PWMCycleStartTime) {
        m_PWMCycleStartTime += m_Period;
        // Check for how long GPIO should be high (at least 100 ms)
        const float pwm_active_ms = pwm_duty_cycle * m_Period / 100.0;
        if (pwm_active_ms >= 100.0)
            gpio_put(m_Pin, true);
        else
            return;
        // Create a non-blocking alarm that triggers upon timeout and turns off the GPIO pin
        // (only if duty cycle < 100% else keep on until next cycle)
        if (static_cast<unsigned>(pwm_duty_cycle) < 100) {
            add_alarm_in_ms(
                static_cast<uint32_t>(pwm_active_ms),
                []([[maybe_unused]] alarm_id_t id, void *user_data) -> int64_t {
                    gpio_put(reinterpret_cast<unsigned>(user_data), false);  // turn off when alarm triggers
                    return 0;  // no repeat
                },
                reinterpret_cast<void*>(m_Pin),
                false
            );
        }
    }
}
