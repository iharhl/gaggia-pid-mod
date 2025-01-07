#include "pid.h"
#include "spi.h"
#include "i2c.h"
#include "led.h"
#include "max31865.h"
#include "ssd1327.h"
#include "pwm.h"
#include "display_manager.h"
#include "time.h"
#include "assert.h"

#include <pico/stdlib.h>
#include <string>
#include <cstdio>

#define MAX_BREW_TEMP 22 // todo
#define MAX_BOILER_TEMP 140
#define PWM_CYCLE 5000  // pwm cycle length in ms
#define BREW_TEMP_SETPOINT 105 // pid setpoint for brewing


template <typename T>
void print(const char* str, T value) {
  printf("%s: %s\n", str, std::to_string(value).c_str());
}


int main() {
  stdio_init_all();

  // Set up LED on the pico
  LED::init();
  LED::turnOn();

  // Set up MAX31865 and its SPI driver
  SPIDevice spi_device(5,2,4,3,1000*1000);
  MAX31865 temp_sensor(&spi_device, MAX31865_3WIRE, MAX31865_FILT_50HZ);

  // Set up SSD1327 display and its I2C driver. Set up display manager
  // which handles layout-specific update of the display.
  I2CDevice i2c_device(12, 13, 400 * 1000, 0x3D);
  SSD1327 display(&i2c_device, 128, 128);
  DisplayManager gui(&display);

  // Set up temperature PID controller
  PIDController pid(5, 0, 0);
  pid.setOutputLimits(0, 100);  // output is pwm duty cycle [%]

  // Set up PWM signal to the solid-state relay (SSR)
  PWMDriver pwm(11, PWM_CYCLE);
  pwm.setMode(true);

  // Setup error handler
  ErrorHandler error_handler(&pwm, &gui);

  // Start time of boiler heating
  uint64_t start_time = Timer::now_min();

  while(true) {

    // Get temperature reading from MAX31865
    float temp = temp_sensor.readTemperature(TEMP_CALC_PRECISE);
    // print("[INFO] TEMPERATURE", temp);

    // Check if MAX31865 reports faults in temp sensing
    const uint8_t fault = temp_sensor.readFault();
    error_handler.myAssert(!fault, CONTEXT_TEMP_SENSING, fault, ERROR);
    if (fault) { temp = 0; } // set to 0 to avoid triggering overheating error

    // Update display temp
    gui.updateTemperature(temp);

    // Check if temp is too high
    error_handler.myAssert(temp < MAX_BOILER_TEMP, CONTEXT_TEMP_CONTROL, 0x01, ERROR);
    error_handler.myAssert(temp < MAX_BREW_TEMP, CONTEXT_TEMP_CONTROL, 0x02, WARNING);

    // Compute PWM duty cycle
    const float pwm_duty_cycle = pid.compute(BREW_TEMP_SETPOINT, temp);
    // print("[INFO] PID OUTPUT", pwm_duty_cycle);

    // Drive the SSR (based on PID output)
    pwm.drivePin(pwm_duty_cycle);

    // todo: sleep for idk how long
    sleep_ms(50);
  }
}
