#!/bin/sh

set -e
set -x

# TODO: Use CMake to produce bitstream target.
# Possibly produce synthesis, implementation, and bitstream as separate targets?

# TODO: --freq option to nextpnr

rm -f synth.json synth.asc synth.bin

yosys synth.ys | tee yosys_stdout.txt
nextpnr-ice40 --up5k --package sg48 --pcf synth.pcf --json synth.json --asc synth.asc | tee nextpnr_stdout.txt
icepack synth.asc synth.bin | tee icepack_stdout.txt
# iceprog synth.bin  # for development board
