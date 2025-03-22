# Gaggia PID mod

Project to add PID temperature control to my Gaggia Classic espresso machine. Inspired by other similar project, I
wanted to test my skills and implement the mod from scratch all by myself.

## Hardware

The temperature control for Gaggia machine is performed with a use of thermostat. It acts as a switch that cuts off when
the temperature exceeds its rating (107 degC).

Instead, we want to have a temperature sensor that would influence the power applied to the heating rods, providing
smoother control over the water temperature.

So how would we implement it in practice? Let's focus on these:
- Temperature sensor
- Power switch
- Microcontroller board

### Temperature sensing

The requirement is to measure relatively brief changes in temperature, wide range of measurement (~ 0 to 200 degC) and
high accuracy (<1 degC).

Temperature control of a water heater is a typical use case for RTD sensors. Hence, Pt100 RTD was chosen. It fits the
requirements and can be bought with M4 adapter needed to connect the sensor to the boiler.

The caveat here is that in order to get a precise temperature reading from the RTD sensor, precise ADC converter and
amplifier have to be used. Someone ofcourse came up with the solution already - MAX31865. It is "easy-to-use
resistance-to-digital converter optimized for platinum resistance temperature detectors". We just need to connect the
sensor, configure it and voilà - you have the temperature reading.

### Power switch

As the thermostat is removed, we need some other component to switch the power on and off. Mechanical relay is what
first comes to mind. And it is technically an OK choice. We don't need to switch power that fast. However, it is
common to use SSR (solid-state relay) for this application. The ones rated for high power applications are vastly
available and relatively cheap; they are easy to connect to the MCU and allow for very fast switching.

At the end, I bought common 40A rated SSR.

### Microcontroller board

The control loop is almost ready - we have a sensor (RTD) and a plant (SSR that switches power). The last part is the
MCU. There are tons of MCUs that fit the job, as long as they have enough GPIOs and support SPI.

I happened to have a Raspberry Pi Pico laying around which I never used before. So I decided to use that as my
microcontroller board.

### Additional hardware

Things above are the most essential parts. But there are few things missing.

First of all, where would Pi Pico get the power from? While debugging, I can connect via USB but that is
temporary solution. AC/DC converter is needed to steal some power from the machine.

Secondly, it would be nice to observe the temperature reading and the status of the machine - hence a small display
would be handy to have.

## Software

TBD

### Display

I decided to separate the display logic from the basic communication class.

## Resources

Code inspirations:
- https://github.com/adafruit/Adafruit_MAX31865/tree/master
- https://github.com/adafruit/Adafruit_SSD1327/blob/master/Adafruit_SSD1327.cpp
- https://github.com/adafruit/Adafruit-GFX-Library/blob/master/Adafruit_GrayOLED.cpp

Control strategy inspirations:
- https://github.com/shmick/Espresso-PID-Controller/tree/master
- https://www.instructables.com/PID-Temperature-Controller/
- PID integral anti-windup implementation from "PID Control" series by the OG Brian Douglas

MAX31865 setup guide:
- https://learn.adafruit.com/adafruit-max31865-rtd-pt100-amplifier/overview

SSD1327 display setup guide:
- https://learn.adafruit.com/adafruit-grayscale-1-5-128x128-oled-display/overview