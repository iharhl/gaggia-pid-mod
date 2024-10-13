cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_COMPILER_FORCED True)
set(CMAKE_C_COMPILER_FORCED True)

set(PICO_SDK_PATH /Users/ihar/Developer/pico-sdk)
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(gaggia_pid_mod LANGUAGES C CXX ASM)

set(PICO_BOARD pico_w)

pico_sdk_init()

add_executable(main
        src/main.cpp
        src/pid.cpp
)
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(main)

target_link_libraries(main pico_stdlib)

if (PICO_CYW43_SUPPORTED)
    target_link_libraries(main pico_cyw43_arch_none)
endif()

