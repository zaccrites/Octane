"""Build a lookup table for a quarter sine wave function."""

import sys
import math


# The bit depth is actually 19 bits because I only have to store the positive
# part of the waveform in the block RAM. The negative part (for which I need
# the 19th bit) is synthesized on demand.
BIT_DEPTH = 19
NUM_SAMPLES = 8192

# Quarter-wave only!
PERIOD = math.pi / 2.0


def main():
    # TODO: argparse

    MAX_RANGE = 2 ** (BIT_DEPTH - 1)

    for i in range(NUM_SAMPLES // 4):
        # t = i / NUM_SAMPLES
        # y = math.sin(t * PERIOD)
        # yn = int(y * MAX_RANGE)

        phase = 2 * math.pi * (2 * i + 1) / (2 * NUM_SAMPLES)
        # phase = 2 * math.pi * i / NUM_SAMPLES
        y = math.sin(phase)
        yn = int(y * MAX_RANGE)

        # https://stackoverflow.com/a/12946226
        MASK = (2 ** BIT_DEPTH) - 1
        # print(f'{yn & MASK :08x}  // sin(2pi * (2*{i} + 1)/(2*{NUM_SAMPLES})) = sin({phase:.12f}) = {y:.012f}')
        print(f'{yn & MASK :08x}  // sin({phase/math.pi:.12f} * pi) = {y:.012f}')


if __name__ == '__main__':
    sys.exit(main())
