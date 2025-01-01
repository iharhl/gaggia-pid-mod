#include "spi.h"

#include <hardware/gpio.h>
#include <pico/time.h>

SPIDevice::SPIDevice(const uint8_t cspin, const uint8_t sckpin,
                     const uint8_t misopin, const uint8_t mosipin,
                     const uint32_t freq) :
    m_cspin(cspin), m_sckpin(sckpin),
    m_misopin(misopin), m_mosipin(mosipin),
    m_freq(freq), m_spi(spi_default)
{
    // No spi instance specified -> take default
    configure();
}

SPIDevice::SPIDevice(const uint8_t cspin, const uint8_t sckpin,
                     const uint8_t misopin, const uint8_t mosipin,
                     const uint32_t freq, spi_inst_t* spi) :
    m_cspin(cspin), m_sckpin(sckpin),
    m_misopin(misopin), m_mosipin(mosipin),
    m_freq(freq), m_spi(spi)
{
    // If using >1 spi connection, specify spi instance to use
    configure();
}

/* Reset spi driver instance */
void SPIDevice::reset() {
    configure();
}

uint8_t SPIDevice::read8() const {
    // Pull the CS pin low and wait a little to make sure MAX31865 can prepare
    // for communication. Technically the timing of pico sdk is ok (~2 us before
    // clk starts) but this way it is still fast but looks a bit smoother.
    gpio_put(m_cspin, false);
    sleep_us(2);
    // Call read from sdk
    uint8_t buff; // 1 byte buffer
    spi_read_blocking(m_spi, 0, &buff, 1);
    // Again, wait before putting CS pin back to high otherwise it'll be
    // too fast for my liking
    sleep_us(2);
    gpio_put(m_cspin, true);

    return buff;
}

uint16_t SPIDevice::read16() const {
    gpio_put(m_cspin, false);
    sleep_us(2);
    uint16_t buff; // 2 byte buffer
    spi_read16_blocking(m_spi, 0, &buff, 1);
    sleep_us(2);
    gpio_put(m_cspin, true);
    return buff;
}

void SPIDevice::read8(uint8_t* buff, const unsigned len) {
    gpio_put(m_cspin, false);
    sleep_us(2);
    spi_read_blocking(m_spi, 0, buff, len);
    sleep_us(2);
    gpio_put(m_cspin, true);
}

void SPIDevice::write8(const uint8_t data) const {
    gpio_put(m_cspin, false);
    sleep_us(2);
    spi_write_blocking(m_spi, &data, 1);
    sleep_us(2);
    gpio_put(m_cspin, true);
}

void SPIDevice::write16(const uint16_t data) const {
    gpio_put(m_cspin, false);
    sleep_us(2);
    spi_write16_blocking(m_spi, &data, 1);
    sleep_us(2);
    gpio_put(m_cspin, true);
}

void SPIDevice::write8(const uint8_t *data, const unsigned len) const {
    gpio_put(m_cspin, false);
    sleep_us(2);
    spi_write_blocking(m_spi, data, len);
    sleep_us(2);
    gpio_put(m_cspin, true);
}

uint8_t SPIDevice::write8ThenRead8(const uint8_t data) const {
    gpio_put(m_cspin, false);
    sleep_us(2);
    uint8_t buff; // 1 byte buffer
    spi_write_blocking(m_spi, &data, 1);
    spi_read_blocking(m_spi, 0, &buff, 1);
    sleep_us(2);
    gpio_put(m_cspin, true);
    return buff;
}

void SPIDevice::write8ThenRead8(const uint8_t *data, uint8_t *buff, const unsigned len) {
    gpio_put(m_cspin, false);
    sleep_us(2);
    spi_write_read_blocking(m_spi, data, buff, len);
    sleep_us(2);
    gpio_put(m_cspin, true);
}

uint16_t SPIDevice::write8ThenRead16(const uint8_t data) const {
    gpio_put(m_cspin, false);
    sleep_us(2);
    uint8_t buff[2]; // 2 byte buffer
    spi_write_blocking(m_spi, &data, 1);
    spi_read_blocking(m_spi, 0, buff, 2);
    sleep_us(2);
    gpio_put(m_cspin, true);
    return buff[1] + buff[0] * (1<<8);
}

void SPIDevice::configure() {
    // Init spi with driver instance and freq
    spi_init(m_spi, m_freq);
    // Set spi format to mode 1 (clock is low when idle, data is sampled on the falling edge)
    spi_set_format(m_spi, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
    // Set pins to spi mode
    gpio_set_function(m_sckpin, GPIO_FUNC_SPI);
    gpio_set_function(m_misopin, GPIO_FUNC_SPI);
    gpio_set_function(m_mosipin, GPIO_FUNC_SPI);
    // Pull the miso pin high during idle
    gpio_pull_up(m_misopin);
    // Set up chip select (cs) pin as gpio (for manual control; needed as the pico sdk default
    // behavior is not exactly matching MAX31865 specifications)
    gpio_init(m_cspin);
    gpio_set_dir(m_cspin, GPIO_OUT);
    gpio_put(m_cspin, true);
}
