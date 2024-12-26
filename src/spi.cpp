#include "spi.h"
#include <hardware/spi.h>
#include <hardware/gpio.h>

SPIDevice::SPIDevice(const uint8_t cspin, const uint8_t sckpin,
                     const uint8_t misopin, const uint8_t mosipin,
                     const uint32_t freq) :
    m_cspin(cspin), m_sckpin(sckpin),
    m_misopin(misopin), m_mosipin(mosipin),
    m_freq(freq), m_spi(spi_default)
{
    // No spi instance specified -> take default
    spi_init(m_spi, m_freq);
    // Set pins to spi mode
    gpio_set_function(m_cspin, GPIO_FUNC_SPI);
    gpio_set_function(m_sckpin, GPIO_FUNC_SPI);
    gpio_set_function(m_misopin, GPIO_FUNC_SPI);
    gpio_set_function(m_mosipin, GPIO_FUNC_SPI);
}

SPIDevice::SPIDevice(const uint8_t cspin, const uint8_t sckpin,
                     const uint8_t misopin, const uint8_t mosipin,
                     const uint32_t freq, spi_inst_t* spi) :
    m_cspin(cspin), m_sckpin(sckpin),
    m_misopin(misopin), m_mosipin(mosipin),
    m_freq(freq), m_spi(spi)
{
    // If using >1 spi connection, specify spi instance to use
    spi_init(m_spi, m_freq);
    // Set pins to spi mode
    gpio_set_function(m_cspin, GPIO_FUNC_SPI);
    gpio_set_function(m_sckpin, GPIO_FUNC_SPI);
    gpio_set_function(m_misopin, GPIO_FUNC_SPI);
    gpio_set_function(m_mosipin, GPIO_FUNC_SPI);
}

void SPIDevice::init() {
    // Reset spi driver
    spi_init(m_spi, m_freq);
}


uint8_t SPIDevice::readByte() const {
    uint8_t buff[1]; // 1 byte buffer
    spi_read_blocking(m_spi, 0, buff, 1);
    return buff[0];
}

void SPIDevice::readBytes(uint8_t* buff, const unsigned len) {
    spi_read_blocking(m_spi, 0, buff, len);
}

void SPIDevice::writeByte(const uint8_t data) const {
    spi_write_blocking(m_spi, &data, 1);
}

void SPIDevice::writeBytes(const uint8_t *data, const unsigned len) const {
    spi_write_blocking(m_spi, data, len);
}

uint8_t SPIDevice::writeThenReadByte(const uint8_t data) const {
    uint8_t buff[1]; // 1 byte buffer
    spi_write_read_blocking(m_spi, &data, buff, 1);
    return buff[0];
}

void SPIDevice::writeThenReadBytes(const uint8_t *data, uint8_t *buff, const unsigned len) {
    spi_write_read_blocking(m_spi, data, buff, len);
}
