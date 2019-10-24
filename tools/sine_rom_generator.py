"""Build a lookup table for a quarter sine wave function."""

import sys
import math


# The SPRAM is 16 bits wide, but we have to accomodate a synthesized sign
# bit, so we only generate 15 bits here.
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
    MAX_RANGE = 2 ** (BIT_DEPTH - 1)

    for i in range(NUM_SAMPLES // 4):
        # https://zipcpu.com/dsp/2017/08/26/quarterwave.html
        phase = 2 * math.pi * (2 * i + 1) / (2 * NUM_SAMPLES)
        y = math.sin(phase)
        yn = int(y * MAX_RANGE)

        # https://stackoverflow.com/a/12946226
        MASK = (2 ** BIT_DEPTH) - 1
        print(f'{yn & MASK :08x}  // sin({phase/math.pi:.12f} * pi) = {y:.012f}')


if __name__ == '__main__':
    sys.exit(main())
