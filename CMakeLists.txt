cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_COMPILER_FORCED True)
set(CMAKE_C_COMPILER_FORCED True)

set(PICO_BOARD pico)

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
        src/clock.cpp
        src/i2c.cpp
        src/ssd1327.cpp
        src/gui.cpp
        src/error.cpp
        src/pump.cpp
)

pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

target_link_libraries(main
        pico_stdlib
        hardware_spi
        hardware_i2c
)

# Create map/bin/hex files etc.
pico_add_extra_outputs(main)

# Enable all warnings
target_compile_options(main
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

# Add cppcheck
find_program(CPPCHECK cppcheck)
if(CPPCHECK)
    add_custom_command(
            TARGET main
            PRE_BUILD
            COMMAND ${CPPCHECK} src/*.cpp
                --enable=all
                --quiet
                --inconclusive
                --check-level=exhaustive
                --std=c++17
                --suppress=unusedFunction
                --suppress=missingIncludeSystem
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
else()
    message(WARNING "Cppcheck not found")
endif()