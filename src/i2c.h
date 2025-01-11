#ifndef I2C_H
#define I2C_H

#include <cstdint>
#include <hardware/i2c.h>


class I2CDevice {
public:
    explicit I2CDevice(uint8_t sdapin, uint8_t sclpin, uint32_t freq,
        uint8_t addr);
    explicit I2CDevice(uint8_t sdapin, uint8_t sclpin, uint32_t freq,
        uint8_t addr, i2c_inst_t* i2c);
    ~I2CDevice() = default;

    void reset();

    [[nodiscard]] uint8_t read8();
    void read8(uint8_t* buff, unsigned len); // read multiple bytes
    void write8(uint8_t data, bool nostop);
    void write8(const uint8_t* data, unsigned len); // write multiple bytes

    unsigned err = 0;

private:
    i2c_inst_t* m_i2c;
    uint8_t m_sdapin, m_sclpin, m_addr;
    uint32_t m_freq;

    void configure();
};



#endif //I2C_H
