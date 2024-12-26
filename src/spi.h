#ifndef SPI_H
#define SPI_H

#include <cstdint>
#include <hardware/spi.h>
#include <sys/types.h>


class SPIDevice {
public:
    SPIDevice() = default;
    SPIDevice(uint8_t cspin, uint8_t sckpin, uint8_t misopin,
        uint8_t mosipin, uint32_t freq);
    SPIDevice(uint8_t cspin, uint8_t sckpin, uint8_t misopin,
        uint8_t mosipin, uint32_t freq, spi_inst_t* spi);
    ~SPIDevice() = default;

    void init();

    [[nodiscard]] uint8_t readByte() const;
    void readBytes(uint8_t* buff, unsigned len);
    void writeByte(uint8_t data) const;
    void writeBytes(const uint8_t* data, unsigned len) const;
    [[nodiscard]] uint8_t writeThenReadByte(uint8_t data) const;
    void writeThenReadBytes(const uint8_t* data, uint8_t* buff, unsigned len);

private:
    uint8_t m_cspin, m_sckpin, m_misopin, m_mosipin;
    uint32_t m_freq;
    spi_inst_t* m_spi;
};


#endif //SPI_H
