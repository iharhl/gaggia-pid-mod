#include "vsens.h"

#include <hardware/adc.h>
#include <hardware/gpio.h>
#include <pico/time.h>


VsysMonitor::VsysMonitor(const uint8_t pin, const uint8_t channel) :
    m_Pin(pin), m_Channel(channel) {
    configure();
}

float VsysMonitor::readOnce() const {
    if (!isADCConfigured())
        return -1.0; // todo: handle error
    const uint16_t raw_adc = adc_read(); // 12-bit value (0–4095)
    const float v_adc = (raw_adc * 3.3f) / 4095.0f;  // ADC reading to voltage
    return v_adc * 3.0f;  // compensate for 200k/100k voltage divider
}

float VsysMonitor::readAvg(const uint8_t num) const {
    if (!isADCConfigured())
        return -1.0; // todo: handle error
    uint32_t sum = 0; // num <= 255 and raw adc <= 4095 --> no overflow
    for (auto i = 0; i < num; i++) {
        sum += adc_read();
        sleep_us(8); // delay for stability
    }
    const float v_adc = (sum / num) * 3.3f / 4095.0f;  // ADC reading to voltage
    return v_adc * 3.0f;  // compensate for 200k/100k voltage divider
}

bool VsysMonitor::isADCConfigured() const {
    if (adc_get_selected_input() != m_Channel)
        return false;
    // Pico's SDK sets gpio function to NULL for ADC
    if (gpio_get_function(m_Pin) != GPIO_FUNC_NULL)
        return false;
    return true;
}

void VsysMonitor::configure() {
    // Initialize ADC
    adc_init();
    adc_gpio_init(m_Pin);
    adc_select_input(m_Channel);
}
