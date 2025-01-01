#include "pico/stdlib.h"
#include "pid.h"
#include <string>
#include "spi.h"
#include "led.h"
#include "max31865.h"
#include <cstdio>


#define RELAY_PIN 11
#define MAX_BOILER_TEMP 140
#define MAX_BOILER_RUNTIME 30  // in minutes
#define PWM_CYCLE 5000  // pwm cycle length in ms
#define BREW_TEMP_SETPOINT 105 // pid setpoint for brewing


template <typename T>
void print(const char* str, T value) {
  printf("%s: %s\n", str, std::to_string(value).c_str());
}

static inline uint64_t now_ms() { return time_us_64() / 1000; }
static inline uint64_t now_min() { return time_us_64() / (1000 * 1000 * 60); }

void configure_output_pin(const uint8_t pin) {
  gpio_init(pin);
  gpio_set_dir(pin, GPIO_OUT);
  gpio_put(pin, false);
}

// void run_debug_code(MAX31865 temp_sensor) {
//   uint16_t adc_raw = temp_sensor.readRTD();
//   print("RTD raw", adc_raw);
//   sleep_ms(300);
//
//   float temp = temp_sensor.readTemperature(TEMP_CALC_ROUGH);
//   print("Temp rought", temp);
//   sleep_ms(300);
//
//   temp = temp_sensor.readTemperature(TEMP_CALC_PRECISE);
//   print("Temp precise", temp);
//   sleep_ms(300);
//
//   uint8_t fault = temp_sensor.readFault();
//   if (fault) {
//     print("Fault", fault);
//     sleep_ms(1000);
//   }
// }


int main() {
  stdio_init_all();
  configure_output_pin(RELAY_PIN);

  LED::init();
  LED::turnOn();

  SPIDevice spi_device(5,2,4,3,1000*1000);
  MAX31865 temp_sensor(&spi_device, MAX31865_3WIRE, MAX31865_FILT_50HZ);

  PIDController pid(1, 0, 0, 1);
  pid.setOutputLimits(0, 100);  // output is pwm duty cycle [%]

  uint64_t pwm_cycle_start_time = now_ms();
  uint64_t start_time = now_min();

  while(true) {
    float temp = temp_sensor.readTemperature(TEMP_CALC_PRECISE);
    print("[INFO] TEMPERATURE", temp);

    uint8_t fault = temp_sensor.readFault();
    if (fault) {
      print("[WARN] SENSOR FAULT", fault);
      LED::blinkForCycles(500, 5);
    }

    const float pwm_duty_cycle = pid.compute(BREW_TEMP_SETPOINT, temp);
    print("[INFO] PID OUTPUT", pwm_duty_cycle);

    // Starts a new PWM cycle every PWM_CYCLE ms
    if (PWM_CYCLE < now_ms() - pwm_cycle_start_time) {
      pwm_cycle_start_time += PWM_CYCLE;
    }

    // If temperature is too high -> disable relay and move the loop
    // forward without applying PID output (but keep running it in the background)
    if (temp > MAX_BOILER_TEMP) {
      gpio_put(RELAY_PIN, false);
      print("[WARN] TEMPERATURE", temp);
      continue;
    }

    if (start_time > MAX_BOILER_RUNTIME) {
      // todo: implement action
      print("[WARN] RUNTIME IN MIN", start_time);
    }

    // Compute how many ms PWM should stay active in the current cycle and compare
    // that to the number of ms that have passed in the current cycle
    const float pwm_active_ms = pwm_duty_cycle * (PWM_CYCLE / 100.0);
    if (pwm_active_ms > 100 and pwm_active_ms > now_ms() - pwm_cycle_start_time)
      gpio_put(RELAY_PIN, true);
    else
      gpio_put(RELAY_PIN, false);

    // todo: sleep for idk how long
    sleep_ms(50);
  }
}
