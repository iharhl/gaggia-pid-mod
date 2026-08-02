#include "vsens.h"

#include <hardware/adc.h>
#include <hardware/gpio.h>
#include <pico/time.h>

#ifdef WAVESHARE_RP2040_ZERO
// external 22k/10k voltage divider: 10/(22+10) = 0.3125 -> compensation: 1/0.3125 = 3.2
constexpr float kDividerFactor = 3.2f;
#else
// Raspberry Pi Pico internal 200k/100k voltage divider: 100/(200+100) = 0.3333 -> compensation: 3.0
constexpr float kDividerFactor = 3.0f;
#endif


VsysMonitor::VsysMonitor(const uint8_t pin, const uint8_t channel) :
    m_Pin(pin), m_Channel(channel) {
    configure();
}

float VsysMonitor::readOnce() const {
    if (!isADCConfigured())
        return -1.0;
    const uint16_t raw_adc = adc_read(); // 12-bit value (0–4095)
    const float v_adc = (raw_adc * 3.3f) / 4095.0f;
    return v_adc * kDividerFactor;
}

float VsysMonitor::readAvg(const uint8_t samples) const {
    if (!isADCConfigured())
        return -1.0;
    uint32_t sum = 0; // num <= 255 and raw adc <= 4095 --> no overflow
    for (auto i = 0; i < samples; i++) {
        sum += adc_read();
        sleep_us(8); // delay for stability
    }
    const float v_adc = (sum / samples) * 3.3f / 4095.0f;
    return v_adc * kDividerFactor;
}

bool VsysMonitor::isADCConfigured() const {
    if (adc_get_selected_input() != m_Channel)
        return false;
    // Pico SDK sets pin function to FUNC_NULL for ADC
    if (gpio_get_function(m_Pin) != GPIO_FUNC_NULL)
        return false;
    return true;
}

void VsysMonitor::configure() {
    adc_init();
    adc_gpio_init(m_Pin);
    gpio_disable_pulls(m_Pin);
    adc_select_input(m_Channel);
}
