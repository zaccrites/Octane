
# Octane

An 8-operator, 32-voice FM synthesizer in the style of 80's synthesizers
like the Yamaha DX7.


## Background

TODO


## Features

* Eight operators
* 32-voice polyphony
* Fully customizable algorithm: any combination of operator modulation is possible
* Feedback on all operators
* Configurable digital filter for each operator

TODO


## Development

    sudo apt install build-essential cmake python3-venv


### Simulator

    sudo apt install build-essential verilator libsdl2-dev


### Synthesis

This project uses the open-source [IceStorm](http://www.clifford.at/icestorm/)
toolchain to synthesize the design, perform place-and-route, and generate
the bitstream.

    TODO: Instructions for installing/building tools or just create
    a script that does it


### Firmware

    sudo apt install openocd gcc-arm-none-eabi
