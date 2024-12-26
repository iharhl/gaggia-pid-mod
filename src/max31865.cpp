#include "max31865.h"
#include <pico/time.h>


bool MAX31865::init(const max31865_wire_num_e wires, const max31865_filter_fq_e fq) {
    //spi.begin()

    setWires(wires);
    enableBias(false);
    convertModeSelect(MAX31865_MODE_NORM_OFF);
    filterSelect(fq);
    setThresholds(0, 0xFFFF);
    clearFault();

    return true;
}

void MAX31865::setWires(const max31865_wire_num_e wires) {
    uint8_t reg = readRegisterByte(MAX31865_CONFIG_REG);
    if (wires == MAX31865_3WIRE) {
        reg |= MAX31865_CONFIG_WIRE;
    } else {
        // 2 or 4 wire
        reg &= ~MAX31865_CONFIG_WIRE;
    }
    writeRegisterByte(MAX31865_CONFIG_REG, reg);
}

void MAX31865::enableBias(const bool b) {
    uint8_t reg = readRegisterByte(MAX31865_CONFIG_REG);
    if (b) {
        reg |= MAX31865_CONFIG_BIAS; // enable bias
    } else {
        reg &= ~MAX31865_CONFIG_BIAS; // disable bias
    }
    writeRegisterByte(MAX31865_CONFIG_REG, reg);
}

void MAX31865::convertModeSelect(const max31865_convert_mode_e mode) {
    uint8_t reg = readRegisterByte(MAX31865_CONFIG_REG);
    if (mode == MAX31865_MODE_AUTO) {
        reg |= MAX31865_CONFIG_MODE; // enable auto convertion
    } else {
        reg &= ~MAX31865_CONFIG_MODE; // disable auto convertion
    }
    writeRegisterByte(MAX31865_CONFIG_REG, reg);
}

void MAX31865::filterSelect(const max31865_filter_fq_e fq) {
    uint8_t reg = readRegisterByte(MAX31865_CONFIG_REG);
    if (fq == MAX31865_50HZ) {
        reg |= MAX31865_CONFIG_FILT;
    } else {
        reg &= ~MAX31865_CONFIG_FILT;
    }
    writeRegisterByte(MAX31865_CONFIG_REG, reg);
}

void MAX31865::setThresholds(const uint16_t lower, const uint16_t upper) {
    writeRegisterByte(MAX31865_LFAULTLSB_REG, lower & 0xFF);
    writeRegisterByte(MAX31865_LFAULTMSB_REG, lower >> 8);
    writeRegisterByte(MAX31865_HFAULTLSB_REG, upper & 0xFF);
    writeRegisterByte(MAX31865_HFAULTMSB_REG, upper >> 8);
}

void MAX31865::clearFault() {
    uint8_t reg = readRegisterByte(MAX31865_CONFIG_REG);
    reg &= ~0b00101100; // write 0 to D5,D3,D2
    reg |= MAX31865_CONFIG_FAULTSTAT; // write 1 to D1
    writeRegisterByte(MAX31865_CONFIG_REG, reg);
}

uint16_t MAX31865::readRTD() {
    clearFault();
    enableBias(true);
    // Delay to charge input caps after bias was off
    sleep_ms(10);
    // Configure single resistance conversion
    uint8_t reg = readRegisterByte(MAX31865_CONFIG_REG);
    reg |= MAX31865_CONFIG_1SHOT;
    writeRegisterByte(MAX31865_CONFIG_REG, reg);
    // Wait till conversion is complete (probably can be reduced to ~55 ms for 50Hz)
    sleep_ms(65);
    // Read most and least significant bits from RTD registers and combine them
    const uint8_t rtd_msb = readRegisterByte(MAX31865_RTDMSB_REG);
    const uint8_t rtd_lsb = readRegisterByte(MAX31865_RTDLSB_REG);
    uint16_t rtd = rtd_lsb + rtd_msb * (1<<8);
    // Disable bias current again to reduce selfheating.
    enableBias(false);
    // Remove fault (reset most right bit D0)
    rtd >>= 1;

    return rtd;
}



uint8_t MAX31865::readRegisterByte(const uint8_t addr) {
    // TODO
}

void MAX31865::writeRegisterByte(uint8_t addr, uint8_t reg) {
    // TODO
}

