"""Build a lookup table for a quarter sine wave function."""

import sys
import math


BIT_DEPTH = 18
NUM_SAMPLES = 2 * 1024

# BIT_DEPTH = 9
# NUM_SAMPLES = 4 * 1024

# Quarter-wave only!
PERIOD = math.pi / 2.0


def main():
    # TODO: argparse

    MAX_RANGE = 2 ** (BIT_DEPTH - 1)


    # TODO: Make the function lookup table   f(t)=sin(2πt)  instead of f(t)=sin(t)
    # We still only want a quarter period, but using 2pi as a multiplier
    # If that even makes sense.


    print(f'// Values are signed {BIT_DEPTH} bit integers')
    print(f'// There are {NUM_SAMPLES} entries ({BIT_DEPTH * NUM_SAMPLES // 1024} kb)')
    for i in range(NUM_SAMPLES):
        # t = i / NUM_SAMPLES
        # y = math.sin(t * PERIOD)
        # y_offset = 0.5 * (y + 1.0)
        # yn = int(y_offset * MAX_RANGE)
        # # print(i, hex(yn), yn, yn/MAX_RANGE)
        # print(f'{yn:08x}  // sin({t:.012f} * pi/2) = {y:.012f}')

        t = i / NUM_SAMPLES
        y = math.sin(t * PERIOD)
        yn = int(y * MAX_RANGE)

        # https://stackoverflow.com/a/12946226
        MASK = (2 ** BIT_DEPTH) - 1
        print(f'{yn & MASK :08x}  // sin({t:.012f} * pi/2) = {y:.012f}')


if __name__ == '__main__':
    sys.exit(main())
