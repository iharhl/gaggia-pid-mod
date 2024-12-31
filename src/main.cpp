#include "pico/stdlib.h"
#include "pid.h"
#include <string>
#include "spi.h"
#include "max31865.h"
#include <cstdio>


int main() {
  // PIDController mypid(1, 0, 0, 1);
  // float res = mypid.compute(10, 30);

  stdio_init_all();

  SPIDevice spi_device(5,2,4,3,1000*1000);
  MAX31865 temp_sensor(&spi_device, MAX31865_3WIRE, MAX31865_FILT_50HZ);

  while (true) {

    uint16_t adc_raw = temp_sensor.readRTD();
    const char* str = std::to_string(adc_raw).c_str();
    printf("RTD raw: ");
    printf(str);
    printf("\n");
    sleep_ms(300);

    float temp = temp_sensor.readTemperature(TEMP_CALC_ROUGH);
    str = std::to_string(temp).c_str();
    printf("Temp rought: ");
    printf(str);
    printf("\n");
    sleep_ms(300);

    temp = temp_sensor.readTemperature(TEMP_CALC_PRECISE);
    str = std::to_string(temp).c_str();
    printf("Temp precise: ");
    printf(str);
    printf("\n");
    sleep_ms(300);

    // uint8_t fault = temp_sensor.readFault();
    // if (fault) {
    //   printf("Fault: ");
    //   printf(std::to_string(fault).c_str());
    //   printf("\n");
    //   sleep_ms(1000);
    // }
  }
}
