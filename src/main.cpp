#include "pid.h"
#include "spi.h"
#include "i2c.h"
#include "led.h"
#include "max31865.h"
#include "ssd1327.h"
#include "pwm.h"
#include "display_manager.h"
#include "pump.h"
#include "error.h"

#include <pico/stdlib.h>
#include <string>
#include <cstdio>


#define BREW_TEMP_HISTERESIS 2 // todo
#define PWM_CYCLE 2000  // pwm cycle length in ms
#define BREW_TEMP_SETPOINT 24 // TODO: change to 105
#define PUMP_AUTOMATIC_CONTROL 1


template <typename T>
void print(const char* str, T value) {
  printf("%s: %s\n", str, std::to_string(value).c_str());
}


int main() {
  stdio_init_all();

  // Set up LED on the pico
  LED::init();
  LED::turnOn();
  sleep_ms(5000);

  // Set up MAX31865 and its SPI driver
  SPIDevice spi_device(5, 2, 4, 3, 1000*1000);
  MAX31865 temp_sensor(&spi_device, MAX31865_3WIRE, MAX31865_FILT_50HZ);

  // Set up SSD1327 display and its I2C driver. Set up display manager
  // which handles layout-specific update of the display.
  I2CDevice i2c_device(12, 13, 400*1000, 0x3D);
  SSD1327 display(&i2c_device, 128, 128);
  DisplayManager gui(&display);

  // Set up temperature PID controller
  PIDController pid(17, 0, 0);
  pid.setOutputLimits(0, 100);  // output is pwm duty cycle [%]

  // Set up PWM signal to the solid-state relay (SSR)
  PWMDriver pwm(11, PWM_CYCLE);
  pwm.setMode(true);

  // Set up pump control
  Pump pump(20, 21);

  // Setup error handler
  ErrorHandler error_handler(&pwm, &gui);

  while(true) {

    // Get temperature reading from MAX31865
    float temp = temp_sensor.readTemperature();
    print("[INFO] TEMPERATURE", temp);

    // Check if MAX31865 reports faults in temp sensing
    const uint8_t fault = temp_sensor.readFault();
    error_handler.verify(!fault, ERROR_CONTEXT_TEMPSENS, ERROR_CODE_0);
    print("Fault", fault);
    if (fault) {
      temp = 0; // set to 0 to avoid triggering overheating error
      temp_sensor.clearFault();
    } else {
      // Check if temp is not too low. Do it only if no fault report
      // otherwise its set to 0 anyway.
      error_handler.verify(temp > BREW_TEMP_SETPOINT - BREW_TEMP_HISTERESIS,
        ERROR_CONTEXT_TEMPLO, ERROR_CODE_0);
    }

    // Update display temp
    gui.updateTemperature(temp);

    // Check if temp is not too high
    error_handler.verify(temp < BREW_TEMP_SETPOINT + BREW_TEMP_HISTERESIS,
      ERROR_CONTEXT_TEMPHI, ERROR_CODE_I);

    // Compute PWM duty cycle
    const float pwm_duty_cycle = pid.compute(BREW_TEMP_SETPOINT, temp);
    print("[INFO] PID OUTPUT", pwm_duty_cycle);

    // Drive the SSR (based on PID output)
    pwm.drivePin(pwm_duty_cycle);

    // Check for errors in i2c and spi
    error_handler.verify(!i2c_device.err, ERROR_CONTEXT_COMM, ERROR_CODE_0);
    error_handler.verify(!spi_device.err, ERROR_CONTEXT_COMM, ERROR_CODE_1);
    i2c_device.err = spi_device.err = 0;

    // Update state of pump relay based on brew switch.
    // If automatic control not enabled -> always stays in the idle state
    // when the relay is closed.
    if (PUMP_AUTOMATIC_CONTROL) {
      const int s = pump.updateState();
      print("State", s);
    }

    // todo: sleep for idk how long
    sleep_ms(200);
  }
}
