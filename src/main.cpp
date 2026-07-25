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
#include <hardware/watchdog.h>
#include <pico/stdlib.h>
#ifdef DEBUG
#include <string>
#endif //DEBUG


/* Brew control configuration defines */
#define PWM_CYCLE_MS            4000      // pwm cycle length in ms
#define BREW_TEMP_SETPOINT      105
#define BREW_TEMP_HYSTERESIS    2         // for LO/HI status display
#define BREW_PID_KP             9.50
#define BREW_PID_KI             0.45
#define BREW_PID_KD             16.0
/* System protection defines */
#define BOILER_INV_LOW_TEMP     0.0       // invalid temperature reading threshold
#define BOILER_OVH_TEMP         155.0     // boiler overheat temperature
#define BOILER_INV_HIGH_TEMP    190.0     // invalid temperature reading threshold
#define BOILER_MAX_RUNTIME      35        // max boiler runtime with heating on (in min)
#ifdef WAVESHARE_RP2040_ZERO
#define VSYS_MIN_VOLT           3.9       // min VDC voltage allowed (VSYS + 0.8V pre-filter drop)
#define VSYS_MAX_VOLT           6.3       // max VDC voltage allowed
#else
#define VSYS_MIN_VOLT           3.1       // min VSYS voltage allowed
#define VSYS_MAX_VOLT           5.5       // max VSYS voltage allowed
#endif
/* SPI pin configuration defines */
#define SPI_CS_PIN              5
#define SPI_SCK_PIN             2
#define SPI_MOSI_PIN            3
#define SPI_MISO_PIN            4
/* I2C pin configuration defines */
#ifdef WAVESHARE_RP2040_ZERO
#define I2C_SDA_PIN             8
#define I2C_SCL_PIN             9
#else
#define I2C_SDA_PIN             20
#define I2C_SCL_PIN             21
#endif
/* GPIO pin configuration defines */
#define GPIO_SSR_PIN            11
#define GPIO_BREW_SWITCH_PIN    26
/* ADC configuration defines */
#define GPIO_ADC_VDD_PIN        29
#define GPIO_ADC_VDD_CH         3


int main() {
  stdio_init_all();

  // Set up LED on the pico
  LED::init();
  LED::turnOn();

#ifdef DEBUG
  sleep_ms(5000);
  printf("[DBG] Board: ");
#ifdef WAVESHARE_RP2040_ZERO
  printf("Waveshare RP2040-Zero\n");
#else
  printf("Raspberry Pi Pico\n");
#endif //WAVESHARE_RP2040_ZERO
#endif //DEBUG

  // Set up SSD1327 display and its I2C driver. Set up display manager
  // which handles layout-specific update of the display.
  I2CDevice i2c_device(I2C_SDA_PIN, I2C_SCL_PIN, 400*1000, 0x3D);
  SSD1327 display(&i2c_device, 128, 128);
  DisplayManager gui(&display);

  // Setup voltage monitor on VSYS. On Pico 1 it is connected to GPIO29_ADC3.
  const VsysMonitor vsys_adc(GPIO_ADC_VDD_PIN, GPIO_ADC_VDD_CH);

  // Set up MAX31865 and its SPI driver
  SPIDevice spi_device(SPI_CS_PIN, SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, 1000*1000);
  MAX31865 temp_sensor(&spi_device, MAX31865_3WIRE, MAX31865_FILT_50HZ);

  // Set up pump class
  Pump pump(GPIO_BREW_SWITCH_PIN);

  // Enable watchdog (should be updated at least 8 times per PWM cycle).
  // If reboot was done -> blink the status code for some time to notify the user.
  if (watchdog_caused_reboot()) {
    gui.blockingStatusAnnouncement(ERROR_CONTEXT_PROT, ERROR_CODE_0);
    gui.resetStatus(true);
    sleep_ms(2000); // can be removed
  }
  watchdog_enable(PWM_CYCLE_MS / 8, false);

  // Set up temperature PID controller (output is pwm duty cycle)
  PIDController pid(BREW_PID_KP, BREW_PID_KI, BREW_PID_KD);
  pid.enableAntiWindup(-40.0, 75.0);

  // Set up PWM signal to the solid-state relay (SSR)
  PWMDriver pwm(GPIO_SSR_PIN, PWM_CYCLE_MS);

  // Setup error handler
  ErrorHandler error_handler(&pwm, &gui);

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
    error_handler.verify(vsys > 0.0, ERROR_CONTEXT_PROT, ERROR_CODE_1);
    error_handler.verify(vsys <= VSYS_MAX_VOLT, ERROR_CONTEXT_PROT, ERROR_CODE_2);
    error_handler.verify(vsys >= VSYS_MIN_VOLT, ERROR_CONTEXT_PROT, ERROR_CODE_3);
    error_handler.verify(temp < BOILER_OVH_TEMP, ERROR_CONTEXT_PROT, ERROR_CODE_4);
    error_handler.verify(Clock::now_min() <= BOILER_MAX_RUNTIME, ERROR_CONTEXT_PROT, ERROR_CODE_5);

    // Display error if present and disable PWM if needed
    error_handler.act();

    // Compute PWM duty cycle
    const float pwm_duty_cycle = pid.compute(BREW_TEMP_SETPOINT, temp);

    // Drive the SSR (based on the PID output)
    pwm.drivePin(pwm_duty_cycle);

    // Update displayed temperature and brew time
    gui.updateTemperature(static_cast<uint8_t>(roundf(temp)));
    gui.updateShotTime(brewtime);

#ifdef DEBUG
    static unsigned dbg_ticks = 0;
    if (++dbg_ticks % 10 == 0) {
      auto [context, code] = error_handler.get();
      printf("[DBG] T=%3.0fC  PID=%5.1f%%  Vsys=%.2fV  Brew=%us  Err=%d|%d\n",
             temp, pwm_duty_cycle, vsys, brewtime, context, code);
    }
#endif //DEBUG

    // Update watchdog
    watchdog_update();

    // Sleep to make a whole loop execute once in ~100ms (can be removed)
    sleep_ms(23);
  }
}
