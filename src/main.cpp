#include "pid.h"
#include "spi.h"
#include "i2c.h"
#include "led.h"
#include "max31865.h"
#include "ssd1327.h"
#include "pwm.h"
#include "gui.h"
#include "pump.h"
#include "error.h"

#include <pico/stdlib.h>


/* Brew control configuration defines */
#define PWM_CYCLE_MS            2000    // pwm cycle length in ms
#define BREW_TEMP_SETPOINT      107
#define BREW_TEMP_HYSTERESIS    2       // for LO/HI status display
/* SPI pin configuration defines */
#define SPI_CS_PIN              5
#define SPI_SCK_PIN             2
#define SPI_MOSI_PIN            3
#define SPI_MISO_PIN            4
/* I2C pin configuration defines */
#define I2C_SDA_PIN             20
#define I2C_SCL_PIN             21
/* GPIO pin configuration defines */
#define GPIO_SSR_PIN            11
#define GPIO_BREW_SWITCH_PIN    26


int main() {
  stdio_init_all();

  // Set up LED on the pico
  LED::init();
  LED::turnOn();

  // Sleep for 2 sec (can be removed)
  sleep_ms(2000);

  // Set up MAX31865 and its SPI driver
  SPIDevice spi_device(SPI_CS_PIN, SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, 1000*1000);
  MAX31865 temp_sensor(&spi_device, MAX31865_3WIRE, MAX31865_FILT_50HZ);

  // Set up SSD1327 display and its I2C driver. Set up display manager
  // which handles layout-specific update of the display.
  I2CDevice i2c_device(I2C_SDA_PIN, I2C_SCL_PIN, 400*1000, 0x3D);
  SSD1327 display(&i2c_device, 128, 128);
  DisplayManager gui(&display);

  // Set up temperature PID controller
  PIDController pid(10, 0.7, 0.1); // increase D gain?
  pid.enableAntiWindup(0, 100);  // output is pwm duty cycle [%]

  // Set up PWM signal to the solid-state relay (SSR)
  PWMDriver pwm(GPIO_SSR_PIN, PWM_CYCLE_MS);
  pwm.setMode(true);

  // Set up pump class
  Pump pump(GPIO_BREW_SWITCH_PIN);

  // Setup error handler
  ErrorHandler error_handler(&pwm, &gui);

  while(true) {
    // Get temperature reading from MAX31865
    float temp = temp_sensor.readTemperature();

    // Check if MAX31865 reports faults in temp sensing
    const uint8_t fault_code = temp_sensor.readFault();
    error_handler.verify(fault_code == 0xFF, ERROR_CONTEXT_TEMPSENS, fault_code);
    if (fault_code != 0xFF) {
      temp = 0; // set to 0 to avoid triggering overheating error
      temp_sensor.clearFault();
    } else {
      // Check if temperature is not too low.
      // Do it only if no fault report otherwise it's set to 0 anyway.
      error_handler.verify(temp > BREW_TEMP_SETPOINT - BREW_TEMP_HYSTERESIS,
        ERROR_CONTEXT_TEMPLO, ERROR_CODE_0);
    }

    // Update pump state based on the brew switch
    const uint8_t brewtime = pump.updateState();

    // Update displayed temperature and brew time
    gui.updateTemperature(static_cast<uint8_t>(temp));
    gui.updateShotTime(brewtime);

    // Check for errors in I2C and SPI communication
    error_handler.verify(!i2c_device.err, ERROR_CONTEXT_COMM, ERROR_CODE_0);
    error_handler.verify(!spi_device.err, ERROR_CONTEXT_COMM, ERROR_CODE_1);
    i2c_device.err = spi_device.err = 0;

    // Check if temperature is not too high
    error_handler.verify(temp < BREW_TEMP_SETPOINT + BREW_TEMP_HYSTERESIS,
      ERROR_CONTEXT_TEMPHI, ERROR_CODE_I);

    // Display error if present and disable PWM if needed
    error_handler.act();

    // Compute PWM duty cycle
    const float pwm_duty_cycle = pid.compute(BREW_TEMP_SETPOINT, temp);

    // Drive the SSR (based on the PID output)
    pwm.drivePin(pwm_duty_cycle);

    // Sleep to make a whole loop execute once in ~100ms (can be removed)
    sleep_ms(23);
  }
}
