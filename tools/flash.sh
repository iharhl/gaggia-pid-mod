#!/bin/sh

if [ $# -eq 0 ]; then
  echo "Please provide an argument - 'pico-debug'/'pico-release'/'zero-debug'/'zero-release'"
fi

case $1 in
  pico-debug)
    echo "=== Building Debug FW..."
    cmake -S . -B cmake-build-debug -DPICO_BOARD=pico -DCMAKE_BUILD_TYPE=Debug
    cmake --build cmake-build-debug
    echo "=== Size of the generated elf:"
    arm-none-eabi-size cmake-build-debug/main.elf
    echo "=== Final size of the binary:"
    stat -f "%N: %z bytes" cmake-build-debug/main.uf2
    echo "=== Flashing debug binary..."
    /Users/ihar/Developer/picotool/build/picotool load -f cmake-build-debug/main.uf2
    ;;
  pico-release)
    echo "=== Building Release FW..."
    cmake -S . -B cmake-build-release -DPICO_BOARD=pico -DCMAKE_BUILD_TYPE=Release
    cmake --build cmake-build-release
    echo "=== Size of the generated elf:"
    arm-none-eabi-size cmake-build-release/main.elf
    echo "=== Final size of the binary:"
    stat -f "%N: %z bytes" cmake-build-release/main.bin
    echo "=== Flashing release binary..."
    /Users/ihar/Developer/picotool/build/picotool load -f cmake-build-release/main.bin
    ;;
  zero-debug)
    echo "=== Building Debug FW..."
    cmake -S . -B cmake-build-zero-debug -DPICO_BOARD=waveshare_rp2040_zero -DCMAKE_BUILD_TYPE=Debug
    cmake --build cmake-build-zero-debug
    echo "=== Size of the generated elf:"
    arm-none-eabi-size cmake-build-zero-debug/main.elf
    echo "=== Final size of the binary:"
    stat -f "%N: %z bytes" cmake-build-zero-debug/main.bin
    echo "=== Flashing debug binary..."
    /Users/ihar/Developer/picotool/build/picotool load -f cmake-build-zero-debug/main.uf2
    ;;
  zero-release)
    echo "=== Building Release FW..."
    cmake -S . -B cmake-build-zero-release -DPICO_BOARD=waveshare_rp2040_zero -DCMAKE_BUILD_TYPE=Release
    cmake --build cmake-build-zero-release
    echo "=== Size of the generated elf:"
    arm-none-eabi-size cmake-build-zero-release/main.elf
    echo "=== Final size of the binary:"
    stat -f "%N: %z bytes" cmake-build-zero-release/main.bin
    echo "=== Flashing release binary..."
    /Users/ihar/Developer/picotool/build/picotool load -f cmake-build-zero-release/main.bin
    ;;
  *)
    echo "Invalid argument - use 'debug' or 'release'"
    exit 1
    ;;
esac