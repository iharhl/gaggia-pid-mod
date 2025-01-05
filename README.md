# Gaggia PID mod

TBD

## Hardware

TBD

## Software

TBD

### Display

I decided to separate the display logic from the basic communication class.

## Resources

Code inspirations:
- https://github.com/adafruit/Adafruit_MAX31865/tree/master
- https://github.com/adafruit/Adafruit_SSD1327/blob/master/Adafruit_SSD1327.cpp
- https://github.com/adafruit/Adafruit-GFX-Library/blob/master/Adafruit_GrayOLED.cpp
- Raspberry Pi documentation for RP2040 MCU itself and its SDK

Control strategy inspirations:
- https://github.com/shmick/Espresso-PID-Controller/tree/master
- https://www.instructables.com/PID-Temperature-Controller/
- PID integral anti-windup implementation from "PID Control" series by the OG Brian Douglas

MAX31865 setup guide:
- https://learn.adafruit.com/adafruit-max31865-rtd-pt100-amplifier/overview

SSD1327 display setup guide:
- https://learn.adafruit.com/adafruit-grayscale-1-5-128x128-oled-display/overview