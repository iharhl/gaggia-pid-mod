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

#include <cmath>
#include <pico/stdlib.h>


/* Brew control configuration defines */
#define PWM_CYCLE_MS            4000    // pwm cycle length in ms
#define BREW_TEMP_SETPOINT      105
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

  // Set up temperature PID controller (output is pwm duty cycle)
  PIDController pid(9.0, 0.4, 16.0);
  pid.enableAntiWindup(-80.0, 80.0);

  // Set up PWM signal to the solid-state relay (SSR)
  PWMDriver pwm(GPIO_SSR_PIN, PWM_CYCLE_MS);

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

    // Handle faults in the temperature reading
    if (fault_code != 0xFF or temp < 0.0 or temp > 255.0) {
      temp = 0.0;
      temp_sensor.clearFault();
    } else {
      // Check if temperature is not too low.
      // Do it only if no fault report otherwise it's set to 0 anyway.
      error_handler.verify(temp > BREW_TEMP_SETPOINT - BREW_TEMP_HYSTERESIS,
        ERROR_CONTEXT_TEMPLO, ERROR_CODE_0);
    }

    // Check for errors in SPI communication
    error_handler.verify(spi_device.isConnected(), ERROR_CONTEXT_COMM, ERROR_CODE_1);

    // Update pump state based on the brew switch
    const uint8_t brewtime = pump.updateState();

    // Check if temperature is not too high
    error_handler.verify(temp < BREW_TEMP_SETPOINT + BREW_TEMP_HYSTERESIS,
      ERROR_CONTEXT_TEMPHI, ERROR_CODE_I);

    // Display error if present and disable PWM if needed
    error_handler.act();

    // Compute PWM duty cycle
    const float pwm_duty_cycle = pid.compute(BREW_TEMP_SETPOINT, temp);

    // Drive the SSR (based on the PID output)
    pwm.drivePin(pwm_duty_cycle);

    // Update displayed temperature and brew time
    gui.updateTemperature(static_cast<uint8_t>(roundf(temp)));
    gui.updateShotTime(brewtime);

    // Check for errors in I2C communication
    error_handler.verify(i2c_device.isConnected(), ERROR_CONTEXT_COMM, ERROR_CODE_0);

    // Sleep to make a whole loop execute once in ~100ms (can be removed)
    sleep_ms(23);
  }
}
