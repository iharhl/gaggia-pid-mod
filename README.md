# Gaggia PID mod

Project to add PID temperature control to my Gaggia Classic espresso
machine. Inspired by other similar project, I wanted to test my skills
and implement the mod from scratch all by myself.

## Hardware (TODO)

The temperature control for Gaggia machine is performed with a use of
thermostat. It acts as a switch that cuts off when the temperature
exceeds its rating (107 degC).

Instead, we want to have a temperature sensor that would influence the
power applied to the heating rods, providing smoother control over the
water temperature.

So how would we implement it in practice? Let's focus on these:
- Temperature sensor
- Power switch
- Microcontroller board

### Temperature sensing (TODO)

The requirement is to measure relatively brief changes in temperature,
wide range of measurement (~ 0 to 200 degC) and high accuracy (<1 degC).

Temperature control of a water heater is a typical use case for RTD
sensors. Hence, Pt100 RTD was chosen. It fits the requirements and can
be bought with M4 adapter needed to connect the sensor to the boiler.

The caveat here is that in order to get a precise temperature reading
from the RTD sensor, precise ADC converter and amplifier have to be used.
Someone ofcourse came up with the solution already — MAX31865. It is
"easy-to-use resistance-to-digital converter optimized for platinum
resistance temperature detectors". We just need to connect the
sensor, configure it and voilà - you have the temperature reading.

### Power switch (TODO)

As the thermostat is removed, we need some other component to switch the
power on and off. Mechanical relay is what first comes to mind. And it
is technically an OK choice. We don't need to switch power that fast.
However, it is common to use SSR (solid-state relay) for this application.
The ones rated for high power applications are vastly available and
relatively cheap; they are easy to connect to the MCU and allow for 
very fast switching.

At the end, I bought a common 40A rated SSR.

### Microcontroller board (TODO)

The control loop is almost ready - we have a sensor (RTD) and a plant
(SSR that switches power). The last part is the MCU. There are tons of
MCUs that fit the job, as long as they have enough GPIOs and support SPI.

I happened to have a Raspberry Pi Pico lying around which I never used
before. So I decided to use that as my microcontroller board.

### Additional hardware (TODO)

Things above are the most essential parts. But there are few things
missing.

First of all, where would Pi Pico get the power from? While debugging,
I can connect via USB but that is temporary solution. AC/DC converter
is needed to steal some power from the machine.

Secondly, it would be nice to observe the temperature reading and the
status of the machine — hence a small display would be handy to have.

### Pump (TODO)

Regarding the hydraulics upgrade, the most popular mod is the dimmer mod.
By chopping the AC supply to the pump, one can control the flow. However,
I read that there is an additional wear on the pump as it is not designed
to work in such conditions.

One simple mod that can be done though, is the pump time control. If one
can interrupt the power to the pump, programmable shot timing can be
achieved.

The only component you need is the relay board...


## Software

Since the Pico board was chosen, the MCU provides plenty of resources to
write a somewhat abstract code :)

To make the implementation easier to write and more readable, I followed
an OOP approach. Each component with a specific purpose is encapsulated
in its own class. For example, each communication protocol (I2C/SPI) is
implemented as a separate class. The drivers for external components,
such as MAX31865 and the OLED display, are also organized into classes
that utilize the I2C or SPI class instances for communication.
Let's break up down below.

### Temperature sensor

Temperature measurements are obtained via the MAX31865 chip, which
uses SPI for communication. Hence, the SPI driver class was implemented.
Due to specific timing requirements of the MAX31865, slight modification
were done to the native Pico SDK SPI calls. The details are available in 
the implementation file — [spi.cpp](src/spi.cpp). Below are examples of
SPI read and write operations captured with a logic analyzer:

<img src="/docs/pulseview-recording/spi-read.png" alt="SPI-read" width="420" height="200">
<img src="/docs/pulseview-recording/spi-write.png" alt="SPI-write" width="420" height="200">

The MAX31865 driver implements the chip-specific logic
and utilizes the SPI class instance for communication. Details can be
found here — [max31865.cpp](src/max31865.cpp)

### PID controller

Not much to explain here — a PID temperature controller logic is
encapsulated within the PID class. The implementation is minimal, only
includes basic P-I-D action, no filtering, and includes integral
anti-windup to prevent the accumulation of integral action (which is 
important in such slow control systems). 

The PID compute method takes temperature setpoint and the measurement as
inputs (error is calculated inside the class), and outputs the PWM
percentage (duty cycle). Implementation can be found here —
[pid.cpp](src/pid.cpp)

### Relay

The SSR is controlled via PWM, hence the logic is put into PWM class.
The small problem here is that the water heating systems are quite slow
leading to a long PWM period, too long to be controlled using Pico's
dedicated timer peripherals. Therefore, the PWM logic was implemented
via GPIO actions. Check details here - [pwm.cpp](src/pwm.cpp)

An interesting consideration here is how much power is delivered to the
heating element during each PWM period. Here's what I mean: suppose
the PID controller outputs a 10% duty cycle. Depending on the length
of the PWM period, this 10% can represent different chunks of time —
for example, 0.11 sec.

In the EU, the AC mains frequency is 50 Hz, meaning each full AC cycle
lasts 20 ms. However, 0.11 sec is not an even multiple of
the AC period (0.11 ÷ 0.02 = 5.5 cycles), so the PWM period slices
through the AC waveform at arbitrary points. As a result, depending on
exactly where in the AC cycle the PWM turns on or off, the average power
delivered can fluctuate between periods.

To achieve more consistent power delivery, it is desirable to align the
PWM period so that it is an integer multiple of the AC cycle period — for
example, setting the PWM period to 2 seconds (100 full AC cycles). This
way, for any given duty cycle, the on-time will always correspond to a
whole number of complete AC periods, rather than cutting through the AC
waveform at arbitrary points. As a result, the average power delivered
becomes more predictable and stable.

At the end of the day, it is not a crucial detail for this system in my
opinion, just an interesting topic to think about.

### Error handler

As this mod is not bulletproof and definitely does not meet industry
standards, some form of error handler was logical to implement. The
solution is straightforward: it checks for a supplied input condition.
If the condition is false and severe enough — disable the PWM and
display the error code. Check implementation here —
[error.cpp](src/error.cpp)

Some improvement would be nice to have, like managing the priorities of
errors. Maybe something for the future...

### Display

The OLED display uses SSD1327 driver chip with I2C as the default
communication protocol. I chose to structure the display logic in
3 layers:

- The I2C driver class handles communication. The default timings in the
Pico SDK were sufficient, so no modifications were necessary. Check 
here — [i2c.cpp](src/i2c.cpp)

- The SSD1327 driver manages the logic for updating the display, such
as filling or clearing the screen, drawing specific regions, and more.
Implementation can be found here — [ssd1327.cpp](src/ssd1327.cpp)

- The GUI driver oversees the display layout, including how quickly the
display is updated and how individual numbers or letters correspond to
an actual pixel data. Source file — [gui.cpp](src/display_manager.cpp)

### Pump

The pump logic is based on a simple state machine. When the brew button
is pressed, the normally closed (NC) relay remains inactive, allowing
the water to flow. After the programmed time elapses, the relay opens,
cutting the power to the pump. After the brew button is released by the
user, the relay returns to the default closed position. For details, see
— [pump.cpp](src/pump.cpp)

This setup enables precise shot timing without the need for manual
tracking of time. While I haven't set up the hardware myself, I've
left the code in place in case I decide to connect it in the future.

### Others (TODO)

Explain clock, led and myprint...

### Utils (TODO)

Explain python helper files...


## Results (TODO)

...

## Resources (TODO)

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