#include "i2c.h"

#include <cstdio> // todo: remove
#include <hardware/gpio.h>


I2CDevice::I2CDevice(const uint8_t sdapin, const uint8_t sclpin,
                    const uint32_t freq, const uint8_t addr)
                    : m_i2c(i2c_default), m_sdapin(sdapin),
                    m_sclpin(sclpin), m_addr(addr), m_freq(freq)
{
    configure();
}

I2CDevice::I2CDevice(const uint8_t sdapin, const uint8_t sclpin,
                    const uint32_t freq, const uint8_t addr, i2c_inst_t* i2c)
                    : m_i2c(i2c), m_sdapin(sdapin),
                    m_sclpin(sclpin), m_addr(addr), m_freq(freq)
{
    configure();
}

/* Reset i2c driver instance */
void I2CDevice::reset() {
    configure();
}

uint8_t I2CDevice::read8() const {
    uint8_t buff;
    i2c_read_blocking(m_i2c, m_addr, &buff, 1, false);
    return buff;
}

void I2CDevice::read8(uint8_t *buff, unsigned len) {
    i2c_read_blocking(m_i2c, m_addr, buff, len, false);
}

uint16_t I2CDevice::read16() const {
    uint8_t buff[2];
    i2c_read_blocking(m_i2c, m_addr, buff, 2, false);
    return buff[1] + buff[0] * (1<<8);
}

// void I2CDevice::write16(uint16_t data) {
// }

void I2CDevice::write8(const uint8_t data, const bool nostop) {
    i2c_write_blocking(m_i2c, m_addr, &data, 1, nostop);
}

void I2CDevice::write8(const uint8_t *data, const unsigned len) {
    i2c_write_blocking(m_i2c, m_addr, data, len, false);
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

