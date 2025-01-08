cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_COMPILER_FORCED True)
set(CMAKE_C_COMPILER_FORCED True)

set(PICO_BOARD pico_w)

set(PICO_SDK_PATH /Users/ihar/Developer/pico-sdk)
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(gaggia_pid_mod LANGUAGES C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

add_executable(main
        src/main.cpp
        src/pid.cpp
        src/max31865.cpp
        src/spi.cpp
        src/led.cpp
        src/pwm.cpp
        src/time.cpp
        src/i2c.cpp
        src/ssd1327.cpp
        src/display_manager.cpp
        src/error.cpp
)

pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

target_link_libraries(main
        pico_stdlib
        pico_cyw43_arch_none
        hardware_spi
        hardware_i2c
)

# create map/bin/hex file etc.
pico_add_extra_outputs(main)