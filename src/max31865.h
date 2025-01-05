#ifndef MAX31865_H
#define MAX31865_H

#include <cstdint>
#include "spi.h"

#define MAX31865_CONFIG_REG 0x00
#define MAX31865_CONFIG_BIAS (1<<7)
#define MAX31865_CONFIG_MODE (1<<6)
#define MAX31865_CONFIG_1SHOT (1<<5)
#define MAX31865_CONFIG_WIRE (1<<4)
#define MAX31865_CONFIG_FAULTSTAT (1<<1)
#define MAX31865_CONFIG_FILT (1<<0)

#define MAX31865_RTDMSB_REG 0x01
#define MAX31865_RTDLSB_REG 0x02
#define MAX31865_HFAULTMSB_REG 0x03
#define MAX31865_HFAULTLSB_REG 0x04
#define MAX31865_LFAULTMSB_REG 0x05
#define MAX31865_LFAULTLSB_REG 0x06
#define MAX31865_FAULTSTAT_REG 0x07

// Polynomial coefficients as shown in
// https://www.analog.com/media/en/technical-documentation/application-notes/AN709_0.pdf
#define RTD_A 3.9083e-3
#define RTD_B -5.775e-7


typedef enum max31865_fault_code {
    MAX31865_FAULT_OVUV = 0x04,
    MAX31865_FAULT_RTDINLOW = 0x08,
    MAX31865_FAULT_REFINHIGH = 0x10,
    MAX31865_FAULT_REFINLOW = 0x20,
    MAX31865_FAULT_LOWTHRESH = 0x40,
    MAX31865_FAULT_HIGHTHRESH = 0x80,
} max31865_fault_code_e;

typedef enum max31865_convert_mode {
    MAX31865_MODE_NORM_OFF = 0,
    MAX31865_MODE_AUTO,
} max31865_convert_mode_e;

typedef enum max31865_wire_num {
    MAX31865_2WIRE = 0,
    MAX31865_3WIRE = 1,
    MAX31865_4WIRE = 0,
} max31865_wire_num_e;

typedef enum max31865_filter_fq {
    MAX31865_FILT_60HZ = 0,
    MAX31865_FILT_50HZ,
} max31865_filter_fq_e;

typedef enum max31865_fault_cycle {
    MAX31865_FAULT_NONE = 0,
    MAX31865_FAULT_AUTO,
    MAX31865_FAULT_MANUAL_RUN,
    MAX31865_FAULT_MANUAL_FINISH
} max31865_fault_cycle_e;

typedef enum temp_calc {
    TEMP_CALC_ROUGH = 0,
    TEMP_CALC_PRECISE,
} temp_calc_e;


class MAX31865 {
public:
    explicit MAX31865(SPIDevice* spi_device, max31865_wire_num_e wires, max31865_filter_fq_e fq);
    ~MAX31865() = default;

    void reset();

    void configureRTD(float RTDnominal, float refResistor);
    [[nodiscard]] uint16_t readRTD();
    void clearFault();
    [[nodiscard]] uint8_t readFault(max31865_fault_cycle_e fault_cycle = MAX31865_FAULT_AUTO);
    float readTemperature(temp_calc_e calcType);

    void setWires(max31865_wire_num_e wires);
    void enableBias(bool b);
    void filterSelect(max31865_filter_fq_e fq);
    void convertModeSelect(max31865_convert_mode_e mode);
    void setThresholds(uint16_t lower, uint16_t upper);

private:
    SPIDevice* m_spidevice;

    float m_R0 = 100; // default = 100 Ohm
    float m_Rref = 4300; // todo: default = 430 Ohm

    float calculateTempPrecise(uint16_t RTDraw);
    float calculateTempRough(uint16_t RTDraw);

    uint8_t readRegisterByte(uint8_t addr);
    uint16_t readRegisterHWord(uint8_t addr);
    void writeRegisterByte(uint8_t addr, uint8_t data);
};


#endif //MAX31865_H
