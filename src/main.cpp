#include "pico/stdlib.h"
#include "pid.h"
#include <string>
#include "spi.h"
#include "max31865.h"


int main() {
  // PIDController mypid(1, 0, 0, 1);
  // float res = mypid.compute(10, 30);

  stdio_init_all();

  SPIDevice spi_device(5,2,4,3,1000*1000);

  MAX31865 temp_sensor(&spi_device, MAX31865_3WIRE, MAX31865_FILT_50HZ);
  temp_sensor.init();

  while (true) {
    uint16_t res = temp_sensor.readRTD();
    const char* res_str = std::to_string(res).c_str();
    printf(res_str);
    sleep_ms(100);
  }
}
