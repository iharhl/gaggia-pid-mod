#include "pico/stdlib.h"
#include "pid.h"

// Pico W devices use a GPIO on the WIFI chip for the LED,
// so when building for Pico W, CYW43_WL_GPIO_LED_PIN will be defined
#include "pico/cyw43_arch.h"
#include "spi.h"


int main() {
  // constexpr uint LED_PIN = 25;

  // PIDController mypid(1, 0, 0, 1);
  // float res = mypid.compute(10, 30);

  stdio_init_all();
  if (cyw43_arch_init()) {
    printf("Wi-Fi init failed");
    return -1;
  }

  // gpio_init(LED_PIN);
  // gpio_set_dir(LED_PIN, GPIO_OUT);

  const SPIDevice spi_device(5,2,4,3,1000*1000);

  while (true) {
    // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
    // sleep_ms(1000);
    // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);

    // Write the output buffer to MOSI, and at the same time read from MISO.
    // spi_write_read_blocking(spi_default, out_buf, in_buf, BUF_LEN);

    uint8_t d[5] = {0x00, 0x10, 0x20, 0x30, 0x40};
    spi_device.writeBytes(d, 5);
    sleep_ms(100);
  }
}
