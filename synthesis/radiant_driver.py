
import os
import sys
import argparse
from subprocess import Popen, PIPE


# impl_1/octane_impl_1.srf  [synthesis]
# impl_1/octane_impl_1.mrp  [mapping]
# impl_1/octane_impl_1.par  [place and route]
# impl_1/octane_impl_1.tws/twr/tw1  [timing]

# impl_1/octane_impl_1.bgn  [bitstream report]
# impl_1/octane_impl_1.bin  [bitstream file]


# TODO: Drive verilator too. Can keep a single list of
# source files here (or as arguments passed by CMake).
# I might just give up on automatically managing Verilator output files
# and just manually add and remove them as needed.


def run(args):
    cmd = []
    stdin = ''
    p = Popen(cmd, stdin=PIPE, stdout=PIPE, stderr=PIPE)
    stdout, stderr= p.communicate(input=stdin)



def main():
    parser = parser.ArgumentParser()
    args = parser.parse_args()
    return run(args)


if __name__ == '__main__':
    sys.exit(main())
