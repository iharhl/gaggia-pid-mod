#ifndef MAX31865_H
#define MAX31865_H

#include <cstdint>

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
// #define MAX31865_FAULTSTAT_REG 0x07

// #define MAX31865_FAULT_HIGHTHRESH 0x80
// #define MAX31865_FAULT_LOWTHRESH 0x40
// #define MAX31865_FAULT_REFINLOW 0x20
// #define MAX31865_FAULT_REFINHIGH 0x10
// #define MAX31865_FAULT_RTDINLOW 0x08
// #define MAX31865_FAULT_OVUV 0x04

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
    MAX31865_60HZ = 0,
    MAX31865_50HZ,
} max31865_filter_fq_e;


class MAX31865 {
public:
    MAX31865();
    ~MAX31865() = default;

    bool init(max31865_wire_num_e, max31865_filter_fq_e);

    uint16_t readRTD();
    void clearFault();
    // uint8_t readFault(max31865_fault_cycle_t fault_cycle = MAX31865_FAULT_AUTO);
    float getTemperature(float RTDnominal, float refResistor);
    float calculateTemperature(uint16_t RTDraw, float RTDnominal, float refResistor);

    void setWires(max31865_wire_num_e wires);
    void enableBias(bool b);
    void filterSelect(max31865_filter_fq_e fq);
    void convertModeSelect(max31865_convert_mode_e mode);
    void setThresholds(uint16_t lower, uint16_t upper);

private:
    uint8_t readRegisterByte(uint8_t addr);
    void writeRegisterByte(uint8_t addr, uint8_t reg);
};


#endif //MAX31865_H
