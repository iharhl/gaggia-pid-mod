#include "i2c.h"

#include <hardware/gpio.h>


I2CDevice::I2CDevice(const uint8_t sdapin, const uint8_t sclpin,
                    const uint32_t freq, const uint8_t addr)
                    : m_i2c(i2c_default), m_sdapin(sdapin),
                    m_sclpin(sclpin), m_addr(addr), m_freq(freq) {
    configure();
}

I2CDevice::I2CDevice(const uint8_t sdapin, const uint8_t sclpin,
                    const uint32_t freq, const uint8_t addr, i2c_inst_t* i2c)
                    : m_i2c(i2c), m_sdapin(sdapin),
                    m_sclpin(sclpin), m_addr(addr), m_freq(freq) {
    configure();
}

/* Reset i2c driver instance */
void I2CDevice::reset() {
    configure();
    m_err = 0;
}

uint8_t I2CDevice::read8() {
    uint8_t buff;
    const int ret = i2c_read_blocking(m_i2c, m_addr, &buff, 1, false);
    if (ret < 0)
        m_err++;
    return buff;
}

void I2CDevice::read8(uint8_t *buff, unsigned len) {
    const int ret = i2c_read_blocking(m_i2c, m_addr, buff, len, false);
    if (ret != static_cast<int>(len))
        m_err++;
}

void I2CDevice::write8(const uint8_t data, const bool nostop) {
    const int ret = i2c_write_blocking(m_i2c, m_addr, &data, 1, nostop);
    if (ret < 0)
        m_err++;
}

void I2CDevice::write8(const uint8_t *data, const unsigned len) {
    const int ret = i2c_write_blocking(m_i2c, m_addr, data, len, false);
    if (ret != static_cast<int>(len))
        m_err++;
}

void I2CDevice::configure() {
    // Init i2c with driver instance and freq
    i2c_init(m_i2c, m_freq);
    // Set pins to i2c mode
    gpio_set_function(m_sdapin, GPIO_FUNC_I2C);
    gpio_set_function(m_sclpin, GPIO_FUNC_I2C);
    // Pull pins up to keep signal high on idle
    gpio_pull_up(m_sdapin);
    gpio_pull_up(m_sclpin);
}

bool I2CDevice::isConnected() {
    if (m_err) {
        m_err = 0; // reset error count
        return false;
    }
    return true;
}
