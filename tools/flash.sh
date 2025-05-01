#!/bin/sh

if [ $# -eq 0 ]; then
  echo "Please provide an argument - 'debug' or 'release'"
fi

case $1 in
  debug)
    echo "=== Size of the generated elf:"
    arm-none-eabi-size cmake-build-debug/main.elf
    echo "=== Flashing debug binary..."
    /Users/ihar/Developer/picotool/build/picotool load -f cmake-build-debug/main.uf2
    ;;
  release)
    echo "=== Size of the generated elf:"
    arm-none-eabi-size cmake-build-release/main.elf
    echo "=== Final size of the binary:"
    stat -f "%N: %z bytes" cmake-build-release/main.bin
    echo "=== Flashing release binary..."
    /Users/ihar/Developer/picotool/build/picotool load -f cmake-build-release/main.bin
    ;;
  *)
    echo "Invalid argument - use 'debug' or 'release'"
    exit 1
    ;;
esac