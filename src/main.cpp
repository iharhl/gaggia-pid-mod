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
#include "clock.h"
#include "vsens.h"

#include <cmath>
#include "myprint.h"
#include <hardware/watchdog.h>
#include <pico/stdlib.h>


/* Brew control configuration defines */
#define PWM_CYCLE_MS            4000      // pwm cycle length in ms
#define BREW_TEMP_SETPOINT      105
#define BREW_TEMP_HYSTERESIS    2         // for LO/HI status display
/* System protection defines */
#define BOILER_INV_LOW_TEMP     0.0f      // invalid temperature reading threshold
#define BOILER_OVH_TEMP         155.0f    // boiler overheat temperature
#define BOILER_INV_HIGH_TEMP    190.0f    // invalid temperature reading threshold
#define BOILER_MAX_RUNTIME      2400      // max boiler runtime with heating on (in sec)
#define VSYS_MIN_VOLT           3.9f      // min VSYS voltage allowed (nominal 5V assumed)
#define VSYS_MAX_VOLT           5.5f      // max VSYS voltage allowed (nominal 5V assumed)
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

  // Set on-board regulator into PWM mode by pulling GPIO23 high.
  // It is connected to the PS pin of the RT6150B regulator.
  // gpio_init(23);
  // gpio_set_dir(23, GPIO_OUT);
  // gpio_put(23, true);

  // Sleep for 1 sec (can be removed)
  sleep_ms(1000);

  // Setup voltage monitor on VSYS. On Pico 1 it is connected to GPIO29_ADC3.
  const VsysMonitor vsys_adc(29, 3);

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
  pid.enableAntiWindup(-40.0, 80.0);

  // Set up PWM signal to the solid-state relay (SSR)
  PWMDriver pwm(GPIO_SSR_PIN, PWM_CYCLE_MS);

  // Set up pump class
  Pump pump(GPIO_BREW_SWITCH_PIN);

  // Setup error handler
  ErrorHandler error_handler(&pwm, &gui);

  // Enable watchdog (should be updated at least 10 times per PWM cycle).
  // If reboot was done -> blink the status code for some time to notify user.
  if (watchdog_caused_reboot()) {
    gui.blockingStatusAnnouncement(ERROR_CONTEXT_PROT, ERROR_CODE_0);
    gui.resetStatus(true);
    sleep_ms(2000); // can be removed
  }
  watchdog_enable(PWM_CYCLE_MS / 10, false);

  while(true) {

    // Get temperature reading from MAX31865
    float temp = temp_sensor.readTemperature();

    // Check if MAX31865 reports faults in temp sensing
    const uint8_t fault_code = temp_sensor.readFault();
    error_handler.verify(fault_code == 0xFF, ERROR_CONTEXT_TEMPSENS, fault_code);
    error_handler.verify(temp > BOILER_INV_LOW_TEMP and temp < BOILER_INV_HIGH_TEMP,
      ERROR_CONTEXT_TEMPSENS, ERROR_CODE_6);

    // Handle faults in the temperature reading
    if (fault_code != 0xFF or temp <= BOILER_INV_LOW_TEMP or temp >= BOILER_INV_HIGH_TEMP) {
      temp = 0.0;
      temp_sensor.clearFault();
    } else {
      // Check if temperature is not too low.
      // Do it only if no fault report otherwise it's set to 0 anyway.
      error_handler.verify(temp > BREW_TEMP_SETPOINT - BREW_TEMP_HYSTERESIS,
        ERROR_CONTEXT_TEMPLO, ERROR_CODE_0);
    }

    // Check if temperature is not too high
    error_handler.verify(temp < BREW_TEMP_SETPOINT + BREW_TEMP_HYSTERESIS,
      ERROR_CONTEXT_TEMPHI, ERROR_CODE_I);

    // Check for errors in SPI and I2C communication
    error_handler.verify(spi_device.isConnected(), ERROR_CONTEXT_COMM, ERROR_CODE_1);
    error_handler.verify(i2c_device.checkErrors(), ERROR_CONTEXT_COMM, ERROR_CODE_0);

    // Update pump state based on the brew switch
    const uint8_t brewtime = pump.updateState();

    // Check for system protection faults
    const float vsys = vsys_adc.readAvg(4);
    error_handler.verify(vsys >= VSYS_MAX_VOLT, ERROR_CONTEXT_PROT, ERROR_CODE_1);
    error_handler.verify(vsys <= VSYS_MIN_VOLT, ERROR_CONTEXT_PROT, ERROR_CODE_2);
    error_handler.verify(temp < BOILER_OVH_TEMP, ERROR_CONTEXT_PROT, ERROR_CODE_3);
    error_handler.verify(Clock::now_sec() < BOILER_MAX_RUNTIME, ERROR_CONTEXT_PROT, ERROR_CODE_4);

    // Display error if present and disable PWM if needed
    error_handler.act();

    // Compute PWM duty cycle
    const float pwm_duty_cycle = pid.compute(BREW_TEMP_SETPOINT, temp);

    // Drive the SSR (based on the PID output)
    pwm.drivePin(pwm_duty_cycle);

    // Update displayed temperature and brew time
    gui.updateTemperature(static_cast<uint8_t>(roundf(temp)));
    gui.updateShotTime(brewtime);

    // Update watchdog
    watchdog_update();

    // Sleep to make a whole loop execute once in ~100ms (can be removed)
    sleep_ms(23);
  }
}
