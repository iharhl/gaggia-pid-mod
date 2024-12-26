#include "pico/stdlib.h"
// #include "boards/pico_w.h"
#include "pid.h"

// Pico W devices use a GPIO on the WIFI chip for the LED,
// so when building for Pico W, CYW43_WL_GPIO_LED_PIN will be defined
#include "pico/cyw43_arch.h"

#include "hardware/spi.h"


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

  spi_init(spi_default, 1000 * 1000);
  gpio_set_function(2, GPIO_FUNC_SPI);
  gpio_set_function(3, GPIO_FUNC_SPI);
  gpio_set_function(4, GPIO_FUNC_SPI);
  gpio_set_function(5, GPIO_FUNC_SPI);

  // Initialize output buffer
  #define BUF_LEN 0x10
  uint8_t out_buf[BUF_LEN], in_buf[BUF_LEN];
  for (size_t i = 0; i < BUF_LEN; ++i) {
    out_buf[i] = i;
  }

  while (true) {
    // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
    // sleep_ms(1000);
    // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);

    // Write the output buffer to MOSI, and at the same time read from MISO.
    spi_write_read_blocking(spi_default, out_buf, in_buf, BUF_LEN);
    sleep_ms(500);
  }
}
