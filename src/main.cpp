#include "pid.h"
#include "spi.h"
#include "i2c.h"
#include "led.h"
#include "max31865.h"
#include "ssd1327.h"
#include "pwm.h"
#include "display_manager.h"
#include "time.h"

#include <pico/stdlib.h>
#include <string>
#include <cstdio>


#define MAX_BOILER_TEMP 140
#define MAX_BOILER_RUNTIME 30  // in minutes
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
  // todo: implement elapsed time
  PIDController pid(5, 0, 0);
  pid.setOutputLimits(0, 100);  // output is pwm duty cycle [%]

  // Set up PWM signal to the solid-state relay (SSR)
  PWMDriver pwm(11, PWM_CYCLE);
  pwm.setMode(true);

  // Start time of boiler heating
  uint64_t start_time = Timer::now_min();

  while(true) {

    float temp = temp_sensor.readTemperature(TEMP_CALC_PRECISE);
    print("[INFO] TEMPERATURE", temp);

    uint8_t fault = temp_sensor.readFault();
    if (fault) {
      pwm.setMode(false);
      print("[WARN] SENSOR FAULT", fault);
      LED::blinkForCycles(500, 5);
    }

    gui.updateTemperature(temp);

    const float pwm_duty_cycle = pid.compute(BREW_TEMP_SETPOINT, temp);
    print("[INFO] PID OUTPUT", pwm_duty_cycle);

    // If temperature is too high -> disable pwm signal
    if (temp > MAX_BOILER_TEMP) {
      pwm.setMode(false);
      print("[WARN] TEMPERATURE", temp);
    }

    // Disable boiler heating after prolong use time
    if (start_time > MAX_BOILER_RUNTIME) {
      pwm.setMode(false);
      print("[WARN] RUNTIME IN MIN", start_time);
    }

    // Drive the SSR (based on PID output)
    pwm.drivePin(pwm_duty_cycle);

    // todo: sleep for idk how long
    sleep_ms(50);
  }
}
