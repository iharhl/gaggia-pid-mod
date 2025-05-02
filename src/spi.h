#ifndef SPI_H
#define SPI_H

#include <cstdint>
#include <hardware/spi.h>


class SPIDevice {
public:
    explicit SPIDevice(uint8_t cspin, uint8_t sckpin, uint8_t misopin,
        uint8_t mosipin, uint32_t freq);
    explicit SPIDevice(uint8_t cspin, uint8_t sckpin, uint8_t misopin,
        uint8_t mosipin, uint32_t freq, spi_inst_t* spi);
    ~SPIDevice() = default;

    void reset();
    bool isConnected();

    [[nodiscard]] uint8_t read8();
    void read8(uint8_t* buff, unsigned len);  // read multiple bytes
    void write8(uint8_t data);
    void write8(const uint8_t* data, unsigned len);  // write multiple bytes
    [[nodiscard]] uint8_t write8ThenRead8(uint8_t data);
    [[nodiscard]] uint16_t write8ThenRead16(uint8_t data);

private:
    void configure();

    uint8_t m_cspin, m_sckpin, m_misopin, m_mosipin;
    uint32_t m_freq;
    spi_inst_t* m_spi;
};


#endif //SPI_H
