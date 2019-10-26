"""Build a lookup table for a quarter sine wave function."""

import sys
import math


# The SPRAM is 16 bits wide, but we have to accomodate a synthesized sign
# bit, so we only generate 15 (unsigned) bits here.
BIT_DEPTH = 15
NUM_SAMPLES = (16 * 1024) * 4

# Quarter-wave only!
PERIOD = math.pi / 2.0


# TODO
template = '''

#ifndef SINE_TABLE_H
#define SINE_TABLE_H

#include <stdint.h>
#define SINE_TABLE_LENGTH  {length}

#ifdef SINE_TABLE_IMPLEMENTATION

const uint16_t SINE_TABLE[SINE_TABLE_LENGTH] = {{
{entries}
}};

#else
extern const uint16_t SINE_TABLE[SINE_TABLE_LENGTH];
#endif

#endif

'''





def main():
    # TODO: argparse
    MAX_RANGE = 2 ** BIT_DEPTH

    for i in range(NUM_SAMPLES // 4):
        # https://zipcpu.com/dsp/2017/08/26/quarterwave.html
        phase = 2 * math.pi * (2 * i + 1) / (2 * NUM_SAMPLES)
        y = math.sin(phase)
        dydx = math.cos(phase)

        # https://stackoverflow.com/a/12946226
        MASK = (2 ** BIT_DEPTH) - 1
        yn = int(y * MAX_RANGE) & MASK

        display_phase = f'{phase/math.pi:.5f} * pi'
        print(f'{yn:04x}  // y(x) = sin({display_phase}) = {y:.012f}   |   dy/dx = cos({display_phase}) = {dydx:.012f}')


if __name__ == '__main__':
    sys.exit(main())
