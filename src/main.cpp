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

  MAX31865 temp_sensor(&spi_device, MAX31865_3WIRE, MAX31865_50HZ);
  temp_sensor.init();

  while (true) {

    // Write the output buffer to MOSI, and at the same time read from MISO.
    // spi_write_read_blocking(spi_default, out_buf, in_buf, BUF_LEN);

    // uint8_t d[5] = {0x00, 0x10, 0x20, 0x30, 0x40};
    // spi_device.writeBytes(d, 5);

    uint16_t res = temp_sensor.readRTD();
    const char* res_str = std::to_string(res).c_str();
    printf(res_str);
    sleep_ms(100);
  }
}
