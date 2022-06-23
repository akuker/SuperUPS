#!/bin/sh
echo "Going to build!"

pushd stm8_controller
cmake .
make -j
