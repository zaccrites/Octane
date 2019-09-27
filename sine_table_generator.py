"""Build a lookup table for a quarter sine wave function."""

import sys
import math


# The block ram is 16 bits, but we synthesize the 17th bit using the
# phase. The multiplier is 16 bits, though, so we can only store 15 bits
# in the ROM.
BIT_DEPTH = 16
NUM_SAMPLES = 8192

# Quarter-wave only!
PERIOD = math.pi / 2.0


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
