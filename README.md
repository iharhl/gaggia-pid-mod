# Gaggia PID mod

This project adds PID temperature control to my Gaggia Classic
espresso machine. Inspired by similar DIY efforts, I wanted to
challenge myself by designing and implementing the modification
entirely from scratch.

By default, Gaggia uses thermostat-based (on/off) temperature control,
which has noticed deviations from the ideal temperature. A PID controller
offers more precise and consistent temperature regulation compared to the
stock thermostat, which helps improve shot quality.

**Update Jul 2026:** I decided to create a custom "carrier" PCB which is
more compact, robust, and makes wiring more organized. Instead of using the
same microcontroller board, I switched from Pi Pico to RP2040-Zero (has the
same MCU). This readme file was updated accordingly.


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
   - [Calibration](#calibration)
   - [Final assembly (old)](#final-assembly-old-pi-pico-version)
   - [Final assembly (new)](#final-assembly-new-rp2040-zero-version)
   - [Taste Test](#taste-test)
4. [Resources](#resources)


## Hardware

The Gaggia machine regulates temperature using basic thermostat, which
functions as a switch that cuts off power when the water temperature
exceeds its rated threshold – 107°C for brewing. However, this approach
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
are frequently used. For this purpose, a Pt100 RTD sensor was
selected. It meets the accuracy and range specifications and is readily
available with an M4 adapter suitable for mounting to a boiler.

However, achieving accurate temperature readings from an RTD sensor
requires a precise analog-to-digital converter (ADC) and amplification.
Fortunately, this problem has been effectively solved with the MAX31865
– a user-friendly resistance-to-digital converter optimized for platinum
RTDs like the Pt100. With the MAX31865, you simply connect the sensor,
configure the device, and it provides accurate digital temperature
readings.

Adafruit offers high-quality breakout boards based on the MAX31865
chip, along with comprehensive documentation and example code. The sensor
itself I found on AliExpress with integrated M4 screw and of 3-wire type.
See examples of MAX31865 and RTD sensor below.

<p>
<img src="/docs/hardware/max31865_board.jpg" alt="hw_MAX31865" width="300" height="225">
<img src="/docs/hardware/pt100.jpg" alt="hw_PT100" width="225" height="225">
</p>

### Power switching

With the thermostat removed, an alternative method is needed to switch
the power on and off. A mechanical relay is the most straightforward
option and probably could suffice.

However, solid-state relays (SSRs) are often preferred in high-power
scenarios for several practical reasons. While both options are capable
of handling the load, SSRs offer significant advantages:

- Silent operation: unlike mechanical relays that produce an audible
"click" during switching, SSRs operate silently.

- Longer lifespan: SSRs have no moving parts, which drastically reduces
mechanical wear and increases operational longevity, particularly under
frequent switching conditions.

- Faster switching: the ability of SSRs to switch much faster than
mechanical relays.

- Reduced electrical noise: SSRs can be zero-crossing triggered, 
minimizing electrical noise and transients during switching.

Given these benefits, I opted for a commonly available 40A-rated SSR.
These are widely used, reliable, and relatively inexpensive, making
them a solid choice for high-power switching applications. According
to the specifications, the control method is listed as zero-crossing.
The SSR model is shown on the image below.

<img src="/docs/hardware/ssr.jpg" alt="hw_SSR" width="220" height="240">

### Microcontroller board

There are many microcontrollers that could handle this task, as long as
they provide enough GPIO pins, support both I2C and SPI communication,
and have sufficient memory resources.

I had a Raspberry Pi Pico on hand that I hadn’t worked with before, so
I decided to use it as the microcontroller for this project.

**Update Jul 2026:** for a carrier PCB, I switch the board to RP2040-Zero –
both are supported now.

<p>
<img src="/docs/hardware/pipico.jpg" alt="hw_PICO" width="280" height="155">
<img src="/docs/hardware/rp2040zero.jpg" alt="hw_ZERO" width="160" height="155">
</p>

### Additional hardware

The components mentioned above cover the core functionality, but a few
important elements are still missing.

First, the Raspberry Pi Pico needs a power source. While it can be
powered via USB during development and debugging, this is only a
temporary solution. For a standalone setup, an AC/DC power supply is
required to draw power from the machine itself. I found a small module
on AliExpress that outputs up to 600mA at 5V, which is more than
sufficient for powering the Pico and a few peripherals.

The module turned out to be of poor quality. Its output was quite noisy,
and there were grounding issues. While most components in the system
would tolerate these, the MAX31865 is considerably more sensitive. As a
result, it frequently reported faults during temperature readings.

Fortunately, I had an AC/DC power supply from my broken speakers on hand,
so I used it for this project. Adding a pair of decoupling capacitors to
its output (100µF electrolytic and 0.1µF ceramic) resulted in a very smooth
voltage curve. However, the output measured at ~6V, which is around absolute
maximum rating of the on-board regulator. I used a diode to introduce a
voltage drop. Additionally, the power supply discharged very slowly after
the power was cut off. To address this, I soldered in a low-value bleed
resistor to speed up the discharge. In the end, the Pico is powered via
the VSYS pin with a stable ~5.1V supply.

<img src="/docs/hardware/acdc.jpg" alt="hw_ACDC" width="220" height="220">

Second, we need a way to monitor the machine's temperature in real
time. A small display would be a practical addition for this purpose,
allowing for quick and convenient observation of temperature readings.
I came across a very nice 1.5" 128×128 grayscale OLED display from
Adafruit, which includes comprehensive documentation.

<img src="/docs/hardware/oled.jpg" alt="hw_OLED" width="220" height="240">


### Bonus: Brew timer

Regarding the hydraulics upgrade, one of the most popular modifications
is the dimmer mod. This approach controls the pump flow by chopping the
AC supply using a phase-angle dimmer circuit. While it allows for
pressure profiling, it is worth noting that this method can lead to
increased wear on the pump, as this vibration pump is not designed to
operate under such conditions.

A simpler alternative is pump time control. By interrupting power to the
pump, you can implement programmable shot timing – effectively automating
brew durations without modifying flow or pressure directly. All that is
required for this mod is a relay board and some wiring to detect when the
brew button has been pressed.

Since I dragged this project out for too long, I ultimately decided
to skip the relay part of the mod – it would not have made a meaningful
difference for my use case anyway. However, I wired the brew button
to the Pico.

On the EU model of the machine, there is a switch-off board
included. Wires on the left side of the brew switch (in my case, blue and
green ones) are connected to it so that the power-off timer is reset
when brewing. I removed these two wires and shorted them (to make the
board think the brew is on at all time). The available connections can
now be used to detect when the button is pressed.


## Software

Since the Pico board was chosen, the MCU provides plenty of resources to
write a somewhat abstract code :)

To make the implementation easier to write and more readable, I followed
an OOP approach. Each component with a specific purpose is encapsulated
in its own class. For example, each communication protocol (I2C/SPI) is
implemented as a separate class. The drivers for external components,
such as MAX31865 and the OLED display, are also organized into classes
that utilize the I2C or SPI class instances for communication.

Here is a high-level overview of the software:

<img src="/docs/architecture/arch.png" alt="SW-arch" width="380" height="400">

### Temperature measurement

Temperature measurements are obtained via the MAX31865 chip, which
uses SPI for communication. Hence, the SPI driver class was implemented.
Due to specific timing requirements of the MAX31865, slight modification
were done to the native Pico SDK SPI calls. The details are available in 
the implementation file – [src/spi.cpp](src/spi.cpp). Below are examples of
SPI read and write operations captured with a logic analyzer:

<p>
<img src="/docs/recordings/spi-read.png" alt="SPI-read" width="380" height="170">
<img src="/docs/recordings/spi-write.png" alt="SPI-write" width="380" height="170">
</p>

The MAX31865 driver implements the chip-specific logic
and utilizes the SPI class instance for communication. Details can be
found here – [src/max31865.cpp](src/max31865.cpp)

### PID controller

Not much to explain here – a PID temperature controller logic is
encapsulated within the PID class. The implementation is minimal, only
includes basic P-I-D action, no filtering, and includes integral
anti-windup to prevent the accumulation of integral action (which is 
important in such slow control systems).

The PID compute method takes temperature setpoint and the measurement as
inputs (error is calculated inside), and outputs the PWM percentage (duty
cycle). Implementation can be found here –
[src/pid.cpp](src/pid.cpp)

### Relay control

The SSR is controlled via PWM, hence the logic is put into PWM class.
The small problem here is that the control systems is quite slow
leading to a long PWM period, too long to be controlled using Pico's
dedicated PWM peripherals. Therefore, the PWM logic was implemented
via combination of timer peripheral and GPIO actions. Check details here –
[src/pwm.cpp](src/pwm.cpp)

<small>

> An interesting consideration here is how much power is delivered to the
heating element during each PWM period. Here's what I mean: suppose
the PID controller outputs a 10% duty cycle. Depending on the length
of the PWM period, this 10% can represent different chunks of time –
for example, 0.11 sec.
> 
> In the EU, the AC mains frequency is 50 Hz, meaning each full AC cycle
lasts 20 ms. However, 0.11 sec is not an even multiple of
the AC period (0.11 / 0.02 = 5.5 cycles), so the PWM period slices
through the AC waveform at arbitrary points. As a result, depending on
exactly where in the AC cycle the PWM turns on or off, the average power
delivered can fluctuate between periods.
>
> To achieve more consistent power delivery, it is desirable to align the
PWM period so that it is an integer multiple of the AC cycle period – for
example, setting the PWM period to 2 seconds (100 full AC cycles). This
way, for any given duty cycle, the on-time will always correspond to a
whole number of complete AC periods, rather than cutting through the AC
waveform at arbitrary points. As a result, the average power delivered
becomes more predictable and stable.
>
> Of course, there are some caveats here, and the topic can get quite
technical – including issues like the non-ideal nature of mains
frequency, inrush current when switching at AC peaks, and other
electrical nuances. That said, these details are not critical for this
particular system in my opinion, especially since the SSR I chose
features zero-crossing control. Still, it's an interesting topic to
explore.

</small>

### Error handler

As this mod is not exactly stress-tested, some form of error handler was
logical to implement.

The approach I took was straightforward: during a cycle, the system
checks whether input conditions are met. If a condition fails and the
associated error has a higher priority than previously stored error, the
system stores the new error code. At the end of the loop, the following
actions are taken:

- Disable PWM output if the error is severe enough.

- Display the current error code. If no error is present, reset the
error status and re-enable the PWM.

Possible error codes are summarized in the table below:

| Error Code | Error Context | Description                                                                           |
|------------|---------------|---------------------------------------------------------------------------------------|
| P0         | Protection    | Watchdog just rebooted the MCU (execution will continue shortly)                      |
| P1         |               | VSYS ADC is not configured correctly                                                  |
| P2         |               | Over voltage detected on VSYS                                                         |
| P3         |               | Under voltage detected on VSYS                                                        |
| P4         |               | Boiler too hot (higher than typical steaming temperature)                             |
| P5         |               | Machine was running for too long                                                      |
| C0         | Communication | Error occurred during I2C communication                                               |
| C1         |               | SPI device seems to be disconnected                                                   |
| S0         | Sensing       | MAX31865 reports under/overvoltage                                                    |
| S1         |               | MAX31865 reports RTD input voltage is too low                                         |
| S2         |               | MAX31865 reports reference input voltage is too high                                  |
| S3         |               | MAX31865 reports reference input voltage is too low                                   |
| S4         |               | MAX31865 reports RTD resistance is lower than the configured low threshold            |
| S5         |               | MAX31865 reports RTD resistance is higher than the configured high threshold          |
| S6         |               | Abnormal / not realistic boiler temperature (without MAX31865 reporting faults)       |
| HI         | User info     | Warning to the user that boiler temperature above setpoint (does not disable the PWM) |
| LO         |               | Warning to the user that boiler temperature below setpoint (does not disable the PWM) |
| OK         |               | Boiler temperature close to setpoint and no errors detected                           |

Check the implementation here – [src/error.cpp](src/error.cpp)

### Display

The OLED display uses SSD1327 driver chip with I2C as the default
communication protocol. I chose to structure the display logic in
3 layers:

- The I2C driver class handles communication. The default timings in the
Pico SDK were sufficient, so no modifications were necessary. Check 
here – [src/i2c.cpp](src/i2c.cpp)

- The SSD1327 driver manages the logic for updating the display, such
as filling or clearing the screen, drawing specific regions, and more.
Implementation can be found here – [src/ssd1327.cpp](src/ssd1327.cpp)

- The GUI driver oversees the display layout, including how quickly the
display is updated and how individual numbers or letters correspond to
an actual pixel data. Source file – [src/gui.cpp](src/gui.cpp)

Image below shows the layout of 128x128 display:

<img src="/docs/display/display-layout.png" alt="disp" width="350" height="350">

It is split into three rows. These rows are:

1. Thermometer icon and three digit fields to show the temperature
of the boiler.
2. Status icon and two fields to show the status code. Error codes are 
represented as a letter and a number (except for HI/LO/OK status codes).
3. Cup icon and two digit fields to show the shot timer. When the brew
switch is pressed, the timer starts. After the button is released, the
timer remains visible for 5 seconds before automatically resetting to
zero. The cup icon changes if the brew timer is active.

Images below show examples of what can be displayed.

<p>
<img src="/docs/display/display-ex-1.png" alt="disp-ex1" width="200" height="200">
<img src="/docs/display/display-ex-2.png" alt="disp-ex2" width="200" height="200">
</p>

<small>

> All the icons, letters, and numbers were drawn by me using an online
> designer for pixel art – Pixilart. The original .pixil files, generated
> png images, etc. are stored in [docs/display/](docs/display)

</small>

### Pump

The pump logic is based on a simple state machine. When the brew button
is pressed, the normally closed (NC) relay remains inactive, allowing
the water to flow. After the programmed time elapses, the relay opens,
cutting the power to the pump. After the brew button is released by the
user, the relay returns to its default closed position. For details, see
– [src/pump.cpp](src/pump.cpp)

This setup enables precise shot timing without the need for manual
tracking of time. While I have not set up the relay part myself, I've
left the code in place.

The part of the code that is used though, is detection of brew button
presses. This enables real-time display of the shot extraction time.

### Other

Source code also includes:

- Clock class provides current uptime of the MCU in ms, sec, and min.
All the methods are static as the class does not need to be instantiated,
it just encapsulates the logic relevant to time. Check
[src/clock.cpp](src/clock.cpp)

- Led class controls the on-board LED. Same as clock, all methods are
static. Check [src/led.cpp](src/led.cpp)

- Vsys monitor class provides the ability to measure the voltage on VSYS line.
As the ADC handling is minimal in the software, I decided to not create a
separate class for it. Check [src/vses.cpp](src/vsens.cpp). **Update Jul 2026:**
custom PCB has a voltage divider, which PR2040-Zero uses to read VDC from
the AC/DC supply, instead of measuring VSYS line itself.

- Watchdog that reboots the MCU if it is not updated in a defined amount
of time. It has no class of its own, direct SDK calls are used.

- For debug and test data recording, custom print functions were
implemented. They communicate data over serial to a connected
computer. Check [src/myprint.h](src/myprint.h)

### Helper tools

To aid the development, I made several scripts which are stored in the
[tools/](tools) folder.

| Script          | Description                                                                     |
|-----------------|---------------------------------------------------------------------------------|
| flash.sh        | Build and flash the binary on the Pico or PR2040-Zero boards and print its size |
| listen.py       | Connect to serial port and record data transmitted from the Pico                |
| process-data.py | Parse and plot the temperature data recorded from serial                        |
| png-to-bytes.py | Convert png image into raw data to be displayed on the OLED                     |


## Results

### Calibration

To calibrate the PID, I began by recording performance data. The
graphs below show the final results I was able to achieve after some
calibration.

The boiler heat-up is quite smooth, reaching operating temperature in
about 100 seconds. The system seems slightly over damped, requiring some
effort to reach the final few degrees. Once at temperature, it oscillates
with a ±1°C deviation for a short while. Given the system's slow thermal
response, I consider this level of accuracy quite acceptable. By around
the 4-minute mark, the error narrows to about ±0.5°C and continues to
improve over time.

The result of a shot pull test is also shown below. During brewing, the
water temperature drops by almost 3°C. After, it overshoots by about
1.5°C but stabilizes relatively quickly, with full recovery achieved
within a minute.

<p>
<img src="/docs/tests/heatup.png" alt="init_heat" width="300" height="225">
<img src="/docs/tests/shot-pull.png" alt="shot_pull" width="300" height="225">
</p>

Additionally, I tested how well my modification works with the stock
steam control. As expected, once the boiler was heated, I reset the
steam switch, and the temperature gradually decreased. After such a
prolonged (passive) cooling, the integral term accumulated significantly,
resulting in an initial temperature undershoot (visible at around the
600-second mark). However, within about one and a half minutes, the
controller compensated and returned the system to the setpoint.

While this is not a realistic usage scenario – doubt anyone would really
wait 10 minutes for the boiler to cool passively (while having the machine
on) – I conducted a more practical test next. After reaching steam
temperature, I manually dropped the temperature by flushing water. This
caused the temperature to fall below the setpoint, but the controller
responded quickly. Two factors contributed to the improved response:
first, the integral term had less time to accumulate error; second, the
steep drop triggered a strong derivative response. In this test, the
system recovered rapidly and stabilized efficiently.

<p>
<img src="/docs/tests/steam-rec-passive.png" alt="steam_act" width="300" height="225">
<img src="/docs/tests/steam-rec-active.png" alt="steam_pas" width="300" height="225">
</p>

Overall, the controller is stable, and the system's performance is more
than adequate for real-world use – substantially better than the stock
configuration. Although further PID tuning is possible, the current
setup delivers great results.

### Final assembly (old Pi Pico version)

The circuit diagram of the final assembly is shown below. It illustrates
how most of the components were wired together.

<img src="/docs/assembly/circuit-old.png" alt="cir-old">

The photos below provide a rough overview of the assembly process. In
the first image, I have highlighted the main modification. Here is a
brief explanation of the numbered tags:

1. **Power Source** – Power is drawn from the machine using piggyback
   connectors, which are active when the main power switch is turned on.
2. **Power Supply Box** – Contains the AC/DC power supply and a small
   piece of protoboard with the necessary components soldered in.
3. **RTD Sensor** – Replaces the machine's original thermostat.
4. **Solid-State Relay (SSR)** – Its AC side is connected to the two
   wires that were previously attached to the brew thermostat.
5. **Brew Switch Wiring** – Two wires are connected to the brew switch
   to detect when it’s pressed. The original disconnected wires are shorted
   to prevent the switch-off board from shutting down the machine.
6. **Control Box** – Houses the Raspberry Pi Pico and the MAX31865.

In the second image, you can see the fully assembled machine. The display
is mounted in a case attached to the side of the machine.

<p>
<img src="/docs/assembly/assembly-old.jpeg" alt="assem-old" width="400" height="534">
<img src="/docs/assembly/machine-old.jpeg" alt="mach-old" width="365" height="534">
</p>

### Final assembly (new RP2040-Zero version)

Updating the assembly in Jul 2026 resulted in a change of a circuit diagram. See
the updated version below. Noticeable differences are:

- The MCU board switched from Pi Pico to RP2040-Zero.
- Voltage divider circuit is no longer on-board – dedicated 22k/10k one is added
  to the custom PCB. Note that it would be better to put it after the capacitors,
  but still works okay as shown.

<img src="/docs/assembly/circuit-new.png" alt="cir-new">

Same as in the previous chapter (old assembly), the photos below show how the new
assembly looks like. In the first image, the main modification are highlighted
with the same number order. The wiring looks way cleaner in the new version.

All the enclosures were updated (for the controller, AC/DC supply and display).
Screen's enclosure is now magnetically attached to the machine's body. I made
hollow standoffs and put few tiny (6x2mm) neodymium magnets there which keep the
screen securely attached to the machine.

<p>
<img src="/docs/assembly/assembly-new.jpeg" alt="assem-new" width="400" height="534">
<img src="/docs/assembly/machine-new.jpeg" alt="mach-new" width="365" height="534">
</p>

### Taste test

Finally, let's talk about the coffee itself. As a not-so-hardcore coffee
enthusiast, here is my take on the stock machine. Even with specialty beans,
it produced only decent results, definitely better than your average
non-specialty cafe, but still nothing I would drink without milk. The espresso
often leaned too acidic, and not in a pleasant way. Once in a while, I would
even get a hardly drinkable shot (skill issue, gonna admit).

The quality of the shots I got while testing the mod were surprising. I
finally was able to taste those flavor "notes" that coffee influencers always
talk about. The acidity was still present, but now it felt balanced – bright,
not sharp – allowing more complex flavors to emerge.

Even more surprising is that in a test setup and very little effort put
into puck preparation, the espresso tasted substantially better than on
the stock machine.


## Resources

Material on how to approach the mod:
- Gaggiuino project - https://gaggiuino.github.io/#/
- BaristaGadgets kit, specifically this video - https://www.youtube.com/watch?v=gj9qLIDaF9g
- Guide on how Gaggia machine works - https://comoricoffee.com/en/gaggia-classic-pro-circuit-diagram-en/

Example code from Adafruit for drivers development:
- MAX31865:
  - https://github.com/adafruit/Adafruit_MAX31865/tree/master
- SSD1327:
  - https://github.com/adafruit/Adafruit_SSD1327/blob/master/Adafruit_SSD1327.cpp
  - https://github.com/adafruit/Adafruit-GFX-Library/blob/master/Adafruit_GrayOLED.cpp

MAX31865 setup guide:
- https://learn.adafruit.com/adafruit-max31865-rtd-pt100-amplifier/overview

SSD1327 display setup guide:
- https://learn.adafruit.com/adafruit-grayscale-1-5-128x128-oled-display/overview