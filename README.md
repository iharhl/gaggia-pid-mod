# Gaggia PID mod

This project adds PID temperature control to my Gaggia Classic
espresso machine. Inspired by similar DIY efforts, I wanted to
challenge myself by designing and implementing the modification
entirely from scratch. 

A PID controller offers more precise and consistent temperature
regulation compared to the stock thermostat, which helps improve shot
quality.


## Table of contents

1. [Hardware](#hardware)
   - [Temperature sensing](#temperature-sensing)
   - [Power switching](#power-switching)
   - [Microcontroller board](#microcontroller-board)
   - [Additional hardware](#additional-hardware)
   - [Bonus: Brew timer](#bonus-brew-timer)
2. [Software](#software)
   - [Temperature measurement](#temperature-measurement)
   - [PID controller](#pid-controller)
   - [Relay control](#relay-control)
   - [Error handler](#error-handler)
   - [Display](#display)
   - [Pump](#pump)
   - [Other](#other)
   - [Helper tools](#helper-tools)
3. [Results](#results)
   - [Final assembly](#final-assembly)
   - [Testing](#testing)
4. [Resources](#resources)


## Hardware

The Gaggia machine regulates temperature using basic thermostat, which
functions as a switch that cuts off power when the water temperature
exceeds its rated threshold — 107°C for brewing. However, this approach
results in a sluggish system response. Once the temperature reaches the
cutoff point, it tends to slowly oscillate around the target, deviating
by several degrees. This rudimentary control method lacks the precision
needed for accurate temperature regulation.

To achieve more precise control, we need to continuously monitor the
water temperature and adjust the power to the heating element dynamically
based on real-time measurements. This allows for smoother and more
accurate temperature control.

To implement such a system, following components are required:
- Temperature sensor
- Power switch
- Microcontroller board

### Temperature sensing

The application requires measuring relatively short-term changes in
temperature over a wide range (~0°C to 200°C) with high accuracy
(better than ±0.5°C).

A common use case that fits these requirements is temperature control
in a water heater, where RTD (Resistance Temperature Detector) sensors
are frequently employed. For this purpose, a Pt100 RTD sensor was
selected. It meets the accuracy and range specifications and is readily
available with an M4 adapter suitable for mounting to a boiler.

However, achieving accurate temperature readings from an RTD sensor
requires a precise analog-to-digital converter (ADC) and amplification.
Fortunately, this problem has been effectively solved with the MAX31865
— a user-friendly resistance-to-digital converter optimized for platinum
RTDs like the Pt100. With the MAX31865, you simply connect the sensor,
configure the device, and it provides accurate digital temperature
readings.

Adafruit offers high-quality breakout boards based on the MAX31865
chip, along with comprehensive documentation and example code. See an
example of such a board in the image below. The sensor itself I found
on AliExpress with integrated M4 screw and of 3-wire type. See an
example in the second image below.

<p>
<img src="/docs/hw/max31865_board.jpg" alt="hw_MAX31865" width="300" height="225">
<img src="/docs/hw/pt100.jpg" alt="hw_PT100" width="225" height="225">
</p>

### Power switching

With the thermostat removed, an alternative method is needed to switch
the power on and off. A mechanical relay is the most straightforward
option and would technically suffice, especially since high-speed
switching is not a hard requirement.

However, solid-state relays (SSRs) are often preferred in high-power
scenarios for several practical reasons. While both options are capable
of handling the load, SSRs offer significant advantages:

- Silent operation: unlike mechanical relays that produce an audible
"click" during switching, SSRs operate silently.

- Longer lifespan: SSRs have no moving parts, which drastically reduces
mechanical wear and increases operational longevity, particularly under
frequent switching conditions.

- Faster switching: while not essential in this case, the ability of
SSRs to switch much faster than mechanical relays.

- Reduced electrical noise: SSRs can be zero-crossing triggered, 
minimizing electrical noise and transients during switching.

Given these benefits, I opted for a commonly available 40A-rated SSR.
These are widely used, reliable, and relatively inexpensive, making
them a solid choice for high-power switching applications. According
to the specifications, the control method is listed as zero-crossing.
The SSR model is shown on the image below.

<img src="/docs/hw/ssr.jpg" alt="hw_SSR" width="220" height="240">

### Microcontroller board

There are many microcontrollers that could handle this task, as long as
they provide enough GPIO pins, support both I2C and SPI communication,
and have sufficient memory resources.

I had a Raspberry Pi Pico on hand that I hadn’t worked with before, so
I decided to use it as the microcontroller for this project.

<img src="/docs/hw/pipico.jpg" alt="hw_PICO" width="280" height="155">

### Additional hardware

The components mentioned above cover the core functionality, but a few
important elements are still missing.

First, the Raspberry Pi Pico needs a power source. While it can be
powered via USB during development and debugging, this is only a
temporary solution. For a standalone setup, an AC/DC converter is
required to draw power from the machine itself. I found a small module
on AliExpress that outputs up to 600mA at 5V, which is more than
sufficient for powering the Pico and a few peripherals.

<img src="/docs/hw/acdc.jpg" alt="hw_ACDC" width="220" height="220">

Second, we need a way to monitor the machine's temperature in real
time. A small display would be a practical addition for this purpose,
allowing for quick and convenient observation of temperature readings.
I came across a very nice 1.5" 128×128 grayscale OLED display from
Adafruit, which includes comprehensive documentation.

<img src="/docs/hw/oled.jpg" alt="hw_OLED" width="220" height="240">


### Bonus: Brew timer

Regarding the hydraulics upgrade, one of the most popular modifications
is the dimmer mod. This approach controls the pump flow by chopping the
AC supply using a phase-angle dimmer circuit. While it allows for
pressure profiling, it is worth noting that this method can lead to
increased wear on the pump, as this vibration pump is not designed to
operate under such conditions.

A simpler and less damaging alternative is pump time control. By
interrupting power to the pump, you can implement programmable shot
timing — effectively automating brew durations without modifying flow
or pressure directly.

All that’s required for this mod is a relay board and some wiring to
detect when the brew button has been pressed.

Since I dragged this project out for too long, I ultimately decided
to skip the relay part of the mod — it would not have made a meaningful
difference for my use case anyway. However, I wired the brew button
to the Pico. On the EU model of the machine, there is a switch-off board
included. Since I do not need its functionality, I removed the wires on
the left side of the brew switch (in my case, blue and green ones).
These connections can now be used to detect when the button is pressed.


## Software

Since the Pico board was chosen, the MCU provides plenty of resources to
write a somewhat abstract code :)

To make the implementation easier to write and more readable, I followed
an OOP approach. Each component with a specific purpose is encapsulated
in its own class. For example, each communication protocol (I2C/SPI) is
implemented as a separate class. The drivers for external components,
such as MAX31865 and the OLED display, are also organized into classes
that utilize the I2C or SPI class instances for communication.

### Temperature measurement

Temperature measurements are obtained via the MAX31865 chip, which
uses SPI for communication. Hence, the SPI driver class was implemented.
Due to specific timing requirements of the MAX31865, slight modification
were done to the native Pico SDK SPI calls. The details are available in 
the implementation file — [src/spi.cpp](src/spi.cpp). Below are examples of
SPI read and write operations captured with a logic analyzer:

<p>
<img src="/docs/recordings/spi-read.png" alt="SPI-read" width="380" height="170">
<img src="/docs/recordings/spi-write.png" alt="SPI-write" width="380" height="170">
</p>

The MAX31865 driver implements the chip-specific logic
and utilizes the SPI class instance for communication. Details can be
found here — [src/max31865.cpp](src/max31865.cpp)

### PID controller

Not much to explain here — a PID temperature controller logic is
encapsulated within the PID class. The implementation is minimal, only
includes basic P-I-D action, no filtering, and includes integral
anti-windup to prevent the accumulation of integral action (which is 
important in such slow control systems). 

The PID compute method takes temperature setpoint and the measurement as
inputs (error is calculated inside the class), and outputs the PWM
percentage (duty cycle). Implementation can be found here —
[src/pid.cpp](src/pid.cpp)

### Relay control

The SSR is controlled via PWM, hence the logic is put into PWM class.
The small problem here is that the water heating systems are quite slow
leading to a long PWM period, too long to be controlled using Pico's
dedicated timer peripherals. Therefore, the PWM logic was implemented
via GPIO actions. Check details here - [src/pwm.cpp](src/pwm.cpp)

> An interesting consideration here is how much power is delivered to the
heating element during each PWM period. Here's what I mean: suppose
the PID controller outputs a 10% duty cycle. Depending on the length
of the PWM period, this 10% can represent different chunks of time —
for example, 0.11 sec.
> 
> In the EU, the AC mains frequency is 50 Hz, meaning each full AC cycle
lasts 20 ms. However, 0.11 sec is not an even multiple of
the AC period (0.11 ÷ 0.02 = 5.5 cycles), so the PWM period slices
through the AC waveform at arbitrary points. As a result, depending on
exactly where in the AC cycle the PWM turns on or off, the average power
delivered can fluctuate between periods.
>
> To achieve more consistent power delivery, it is desirable to align the
PWM period so that it is an integer multiple of the AC cycle period — for
example, setting the PWM period to 2 seconds (100 full AC cycles). This
way, for any given duty cycle, the on-time will always correspond to a
whole number of complete AC periods, rather than cutting through the AC
waveform at arbitrary points. As a result, the average power delivered
becomes more predictable and stable.
>
> Of course, there are some caveats here, and the topic can get quite
technical — including issues like the non-ideal nature of mains
frequency, inrush current when switching at AC peaks, and other
electrical nuances. That said, these details are not critical for this
particular system in my opinion, especially since the SSR I chose
features zero-crossing control. Still, it's an interesting topic to
explore.

### Error handler

As this mod is not bulletproof and definitely does not meet industry
standards, some form of error handler was logical to implement. The
solution is straightforward: it checks for a supplied input condition.
If the condition is false and severe enough — disable the PWM and
display the error code. Check implementation here —
[src/error.cpp](src/error.cpp)

Some improvement would be nice to have, like managing the priorities of
errors. Maybe something for the future...

### Display

The OLED display uses SSD1327 driver chip with I2C as the default
communication protocol. I chose to structure the display logic in
3 layers:

- The I2C driver class handles communication. The default timings in the
Pico SDK were sufficient, so no modifications were necessary. Check 
here — [src/i2c.cpp](src/i2c.cpp)

- The SSD1327 driver manages the logic for updating the display, such
as filling or clearing the screen, drawing specific regions, and more.
Implementation can be found here — [src/ssd1327.cpp](src/ssd1327.cpp)

- The GUI driver oversees the display layout, including how quickly the
display is updated and how individual numbers or letters correspond to
an actual pixel data. Source file — [src/gui.cpp](src/gui.cpp)

Image below shows the layout of 128x128 display. It is split into three
rows. Each row has an indicator of some parameters. In this case:

1. Thermometer icon and three digit fields to show the temperature.
2. Status icon and two fields to show the status code. Error codes are 
represented as a letter and a number. Additional codes are:
   - OK for no errors and boiler reading for brewing.
   - LO for boiler too cold for brewing.
   - HI for boiler too hot for brewing.
3. Cup icon and two digit fields to show the shot timer. When the brew
switch is pressed, the timer starts. After the button is released, the
timer remains visible for 5 seconds before automatically resetting to
zero.

<img src="/docs/display/display-layout.png" alt="disp" width="350" height="350">

### Pump

The pump logic is based on a simple state machine. When the brew button
is pressed, the normally closed (NC) relay remains inactive, allowing
the water to flow. After the programmed time elapses, the relay opens,
cutting the power to the pump. After the brew button is released by the
user, the relay returns to its default closed position. For details, see
— [src/pump.cpp](src/pump.cpp)

This setup enables precise shot timing without the need for manual
tracking of time. While I have not set up the relay part myself, I've
left the code in place.

The part of the code that is used though, is detection of brew button
presses. This enables real-time display of the shot extraction time.

### Other

Source code also includes:

- Clock class provides current uptime of the MCU in ms, sec and min.
All the methods are static as the class does not need to be instantiated,
it just encapsulates the logic relevant to time. Check
[src/clock.cpp](src/clock.cpp)

- Led class controls the on-board LED. Same as clock, all methods are
static. Check [src/led.cpp](src/led.cpp)

- For debug and test data recording, custom print functions were
implemented. They communicate data over serial to a connected
computer. Check [src/myprint.h](src/myprint.h)

### Helper tools

To aid the development, I made several scripts which are stored in the
[tools/](tools) folder.

| Script          | Description                                                      |
|-----------------|------------------------------------------------------------------|
| flash.sh        | Flash the binary on the Pico and print its size                  |
| listen.py       | Connect to serial port and record data transmitted from the Pico |
| process-data.py | Parse and plot the temperature data recorded from serial         |
| png-to-bytes.py | Convert png image into raw data to be displayed on the OLED      |


## Results

### Final assembly

Here is the circuit diagram of the final assembly:

<img src="/docs/architecture/circuit.png" alt="circuit" width="600" height="340">


### Testing

To calibrate the PID, I began by recording performance data. The
graphs below show the results after several calibration attempts.

The boiler heat-up is quite smooth, reaching operating temperature in
about 90 seconds. There's a slight initial overshoot of approximately
1°C, which quickly settles.

I also tested how well my modification works with the stock steam
control. As expected, once the boiler was heated, I reset the steam
switch, and the temperature gradually decreased. At around 150 seconds,
I activated the brew button to circulate water and help cool the boiler,
then reset it after about 20 seconds.

The PID controller managed this transition adequately. The first
temperature peak reached 110°C, quite an overshoot, but the second
peak dropped to an acceptable 108°C.

<p>
<img src="/docs/final-tests/heatup.png" alt="init_heat" width="300" height="225">
<img src="/docs/final-tests/steam-heatup-rec.png" alt="steam_rec" width="300" height="225">
</p>

Recordings of water dump and shot pull tests are shown below. Since
water flow is more restricted during an actual shot, the system performs
is better than during the water dump: temperature dip is smaller, and
overshoot is reduced.

<p>
<img src="/docs/final-tests/water-dump.png" alt="water_dump" width="300" height="225">
<img src="/docs/final-tests/shot-pull.png" alt="shot_pull" width="300" height="225">
</p>

Overall, the control system performs well for real-world use and
outperforms the stock configuration. While the PID can definitely be
tuned further, the current performance is satisfactory to me.


## Resources

Material on how to approach the mod:
- Gagginno project - https://gaggiuino.github.io/#/
- BaristaGadgets kit, specifically this video - https://www.youtube.com/watch?v=gj9qLIDaF9g

Example code for drivers development:
- https://github.com/adafruit/Adafruit_MAX31865/tree/master
- https://github.com/adafruit/Adafruit_SSD1327/blob/master/Adafruit_SSD1327.cpp
- https://github.com/adafruit/Adafruit-GFX-Library/blob/master/Adafruit_GrayOLED.cpp

Control strategy inspirations:
- https://github.com/shmick/Espresso-PID-Controller/tree/master
- https://www.instructables.com/PID-Temperature-Controller/
- PID integral anti-windup implementation from "PID Control" series by Brian Douglas

MAX31865 setup guide:
- https://learn.adafruit.com/adafruit-max31865-rtd-pt100-amplifier/overview

SSD1327 display setup guide:
- https://learn.adafruit.com/adafruit-grayscale-1-5-128x128-oled-display/overview