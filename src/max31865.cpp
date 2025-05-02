#include "max31865.h"
#include "myprint.h"

#include <cmath>
#include <pico/time.h>
#include <string>


MAX31865::MAX31865(SPIDevice* spi_device,
        const max31865_wire_num_e wires,
        const max31865_filter_fq_e fq) : m_spidevice(spi_device)
{
    setWires(wires);
    enableBias(false);
    modeSelect(MAX31865_MODE_NORM_OFF);
    filterSelect(fq);
    setThresholds(0, 0xFFFF);
    clearFault();
}

/* Reset MAX31865 board and its spi driver */
void MAX31865::reset() {
    m_spidevice->reset();
    enableBias(false);
    modeSelect(MAX31865_MODE_NORM_OFF);
    clearFault();
}

void MAX31865::setWires(const max31865_wire_num_e wires) {
    uint8_t reg = readRegisterByte(MAX31865_CONFIG_REG);
    if (wires == MAX31865_3WIRE)
        reg |= MAX31865_CONFIG_WIRE;
    else
        reg &= ~MAX31865_CONFIG_WIRE; // 2 or 4 wire
    writeRegisterByte(MAX31865_CONFIG_REG, reg);
}

void MAX31865::enableBias(const bool b) {
    uint8_t reg = readRegisterByte(MAX31865_CONFIG_REG);
    if (b)
        reg |= MAX31865_CONFIG_BIAS; // enable bias
    else
        reg &= ~MAX31865_CONFIG_BIAS; // disable bias
    writeRegisterByte(MAX31865_CONFIG_REG, reg);
}

void MAX31865::modeSelect(const max31865_convert_mode_e mode) {
    uint8_t reg = readRegisterByte(MAX31865_CONFIG_REG);
    if (mode == MAX31865_MODE_AUTO) {
        reg |= MAX31865_CONFIG_MODE; // enable auto convertion
        reg &= ~MAX31865_CONFIG_1SHOT; // reset 1-shot bit
        reg |= MAX31865_CONFIG_BIAS; // enable bias
    }
    else {
        reg &= ~MAX31865_CONFIG_MODE; // disable auto convertion
    }
    writeRegisterByte(MAX31865_CONFIG_REG, reg);
    m_mode = mode;
}

void MAX31865::filterSelect(const max31865_filter_fq_e fq) {
    uint8_t reg = readRegisterByte(MAX31865_CONFIG_REG);
    if (fq)
        reg |= MAX31865_CONFIG_FILT;
    else
        reg &= ~MAX31865_CONFIG_FILT;
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

uint8_t MAX31865::readFault(const max31865_fault_cycle_e fault_cycle) {
    if (fault_cycle) {
        // Reset config register except wire and filter bits
        uint8_t reg = readRegisterByte(MAX31865_CONFIG_REG);
        reg &= 0b00010001;
        // Initiate fault detection (of type auto or manual)
        switch (fault_cycle) {
            case MAX31865_FAULT_AUTO:
                writeRegisterByte(MAX31865_CONFIG_REG, (reg | 0b10000100));
                sleep_ms(1);
                break;
            case MAX31865_FAULT_MANUAL_RUN:
                // Ensure that Vbias has been on for at least 5 time const
                writeRegisterByte(MAX31865_CONFIG_REG, (reg | 0b10001000));
                return 0;
            case MAX31865_FAULT_MANUAL_FINISH:
                // Wait for at least 5 time const after manual run
                writeRegisterByte(MAX31865_CONFIG_REG, (reg | 0b10001100));
                return 0;
            default:
                break;
        }
    }
    // Read the fault value from fault register
    const uint8_t fault = readRegisterByte(MAX31865_FAULTSTAT_REG);
    // Convert the value into error code for the display
    switch (fault) {
        case MAX31865_FAULT_OVUV: return 0;
        case MAX31865_FAULT_RTDINLOW: return 1;
        case MAX31865_FAULT_REFINHIGH: return 2;
        case MAX31865_FAULT_REFINLOW: return 3;
        case MAX31865_FAULT_LOWTHRESH: return 4;
        case MAX31865_FAULT_HIGHTHRESH: return 5;
        default: return 0xFF;
    }
}

void MAX31865::configureRTD(const float RTDnominal, const float refResistor) {
    // The 'nominal' resistance of the RTD sensor, usually 100 or 1000
    m_R0 = RTDnominal;
    // The value of the matching reference resistor, usually 430 or 4300
    m_Rref = refResistor;
}

uint16_t MAX31865::readRTD() {
    uint16_t rtd = 0;
    // Clear fault from previous read
    clearFault();
    // Read rtd based on the current mode (auto or normally off)
    if (m_mode == MAX31865_MODE_NORM_OFF) {
        enableBias(true);
        // Delay to charge input caps after bias was off
        sleep_ms(10);
        // Configure single resistance conversion
        uint8_t reg = readRegisterByte(MAX31865_CONFIG_REG);
        reg |= MAX31865_CONFIG_1SHOT;
        writeRegisterByte(MAX31865_CONFIG_REG, reg);
        // Wait till conversion is complete (generous 65 ms wait time)
        sleep_ms(65);
        // Read 2 bytes (half-word) from RTD registers
        rtd = readRegisterHWord(MAX31865_RTDMSB_REG);
        // Disable bias current to reduce self-heating
        enableBias(false);
    } else {
        // Read 2 bytes (half-word) from RTD registers
        rtd = readRegisterHWord(MAX31865_RTDMSB_REG);
    }
    // Remove fault (reset most right bit D0)
    rtd >>= 1;
    return rtd;
}

float MAX31865::readTemperature() {
    return calculateTemp(readRTD());
}

float MAX31865::calculateTemp(const uint16_t RTDraw) const {
    // The resistance vs. temperature curve is reasonably linear, but has some curvature.
    // Using Callendar-Van Dusen equation, we can describe that curvature.
    // Check https://www.analog.com/media/en/technical-documentation/application-notes/AN709_0.pdf
    // for details.
    // --------------------------------------------
    // A platinum RTD’s transfer function is described by two distinct polynomial equations:
    // one for temperatures below 0degC and another for temperatures above 0degC.
    // --------------------------------------------
    // These equations are:
    //      R_RTD(t) = R0 * [ 1 + A*t + B*t^2 + C*(t–100)*t^3 ]     (for t <= 0degC)
    //      R_RTD(t) = R0 * [ 1 + A*t + B*t^2 ]                     (for t >= 0degC)
    // ---------------------------------------------
    // In the code below, these are represented as:
    //      t          ->   temp
    //      R_RTD      ->   R
    //      R0         ->   m_R0
    //      A          ->   RTD_A
    //      B          ->   RTD_B
    // --------------------------------------------
    // As per datasheet, in order to convert ADC raw value to RTD resistance,
    // the following equation is used:
    //      R_RTD = (ADCraw * R_REF) / 2^15

    const float R = static_cast<float>(RTDraw) * m_Rref / (1<<15);

    constexpr float Z1 = -RTD_A;
    constexpr float Z2 = RTD_A * RTD_A - (4 * RTD_B);
    const float Z3 = (4 * RTD_B) / m_R0;
    constexpr float Z4 = 2 * RTD_B;

    const float temp = (std::sqrt(Z2 + Z3 * R) + Z1) / Z4;

    // The calculation for < 0 degC is slightly different but as it is not expected
    // for temperature to drop this low, the calculation was removed.
    if (temp < 0) { return 0; }

    return temp;
}

uint8_t MAX31865::readRegisterByte(uint8_t addr) {
    // MSB (A7) is reset (=0) to indicate read operation
    addr &= ~(1<<7);
    return m_spidevice->write8ThenRead8(addr);
}

/* Read half-word (note: arm32 word = 4 bytes) */
uint16_t MAX31865::readRegisterHWord(uint8_t addr) {
    // MSB (A7) is reset (=0) to indicate read operation
    addr &= ~(1<<7);
    return m_spidevice->write8ThenRead16(addr);
}

void MAX31865::writeRegisterByte(uint8_t addr, const uint8_t data) {
    // MSB (A7) is set (=1) to indicate write operation
    addr |= (1<<7);
    // Make write buffer to transfer 2 bytes - addr and data
    const uint8_t buff[2] = {addr, data};
    m_spidevice->write8(buff, 2);
}

