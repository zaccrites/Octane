"""Build a ROM for algorithm control signals.

For each operator, there is a set of control signals which control which
registers are updated with the new waveform value, which value the selector
will choose for modulation, etc.

The Yamaha DX7 implemented 32 algorithms using its six operators,
and so will we.

Signals:
    - SEL (3) : Selects modulation source for next operator.
    - MREN (1) : Write to the M register
    - FREN (1) : Write to the F register

The instructions are stored in an FPGA block ROM addressed by algorithm
number (using 6 bits) and current operator number (using 3 bits),
both zero based.
For example, for Algorithm 6 and Operator 4 the address would be 0x0c5:

    ADDRESS[8:3] = 6'b000101
    ADDRESS[2:0] = 3'b011

Using 6 bits for the algorithm number allows possible expansion to more than 32
algorithms using up to 8 operators, as long as only up to 8 control bits are
required (using an iCE40 512x8 bit RAM configuration).
Otherwise a 256x16 bit configuration is supported.

The control bits are stored as follows:

    WORD[7:5] = <reserved>
    WORD[4:2] = SEL
    WORD[1]   = MREN
    WORD[0]   = FREN

"""

# TODO: Consider emitting another file describing the algorithms,
#   including how many carriers there are (e.g. for deriving the COM factor).

import sys
from enum import IntEnum
from collections import namedtuple


class ModulationSource(IntEnum):
    none = 0
    previous = 1
    mreg = 2
    mreg_plus_previous = 3
    freg = 4


Algorithm = namedtuple('Algorithm', ['name', 'steps'])
Step = namedtuple('Step', ['sel', 'mren', 'fren'])


def make_algorithm(*steps):
    if len(steps) != 6:
        raise ValueError('Algorithm must have six steps')
    if not all(isinstance(step, Step) for step in steps):
        raise TypeError('All steps must be of type Step')
    return list(steps)


def make_step(*, sel, mren=False, fren=False):
    return Step(sel=sel, mren=mren, fren=fren)




def get_algorithms():
    algorithms = []

    """Algorithm 1
             |--|
            [1] |
             |--|
            [2]
             |
        [5] [3]
         |   |
        [6] [4]
         |   |
         +---+
    """
    algorithms.append(make_algorithm(
        make_step(sel=ModulationSource.freg, fren=True),
        make_step(sel=ModulationSource.previous),
        make_step(sel=ModulationSource.previous),
        make_step(sel=ModulationSource.previous, mren=True),
        #
        make_step(sel=ModulationSource.none),
        make_step(sel=ModulationSource.previous, mren=True),
    ))

    """Algorithm 2
            [1]
             |
            [2]
      |--|   |
      | [5] [3]
      |--|   |
        [6] [4]
         |   |
         +---+
    """
    algorithms.append(make_algorithm(
        make_step(sel=ModulationSource.none),
        make_step(sel=ModulationSource.previous),
        make_step(sel=ModulationSource.previous),
        make_step(sel=ModulationSource.previous, mren=True),
        #
        make_step(sel=ModulationSource.freg, fren=True),
        make_step(sel=ModulationSource.previous, mren=True),
    ))

    """Algorithm 3
             |--|
        [4] [1] |
         |   |--|
        [5] [2]
         |   |
        [6] [3]
         |   |
         +---+
    """
    algorithms.append(make_algorithm(
        make_step(sel=ModulationSource.freg, fren=True),
        make_step(sel=ModulationSource.previous),
        make_step(sel=ModulationSource.previous, mren=True),
        #
        make_step(sel=ModulationSource.none),
        make_step(sel=ModulationSource.previous),
        make_step(sel=ModulationSource.previous, mren=True),
    ))

    """Algorithm 4
             |--|
        [4] [1] |
         |   |  |
        [5] [2] |
         |   |  |
        [6] [3] |
         |   +--|
         +---+
    """
    algorithms.append(make_algorithm(
        make_step(sel=ModulationSource.freg),
        make_step(sel=ModulationSource.previous),
        make_step(sel=ModulationSource.previous, mren=True, fren=True),
        #
        make_step(sel=ModulationSource.none),
        make_step(sel=ModulationSource.previous),
        make_step(sel=ModulationSource.previous, mren=True),
    ))

    """Algorithm 5
                 |--|
        [5] [3] [1] |
         |   |   |--|
        [6] [4] [2]
         |   |   |
         +---+---+
    """
    algorithms.append(make_algorithm(
        make_step(sel=ModulationSource.freg, fren=True),
        make_step(sel=ModulationSource.previous, mren=True),
        #
        make_step(sel=ModulationSource.none),
        make_step(sel=ModulationSource.previous, mren=True),
        #
        make_step(sel=ModulationSource.none),
        make_step(sel=ModulationSource.previous, mren=True),
    ))

    """Algorithm 6
                 |--|
        [5] [3] [1] |
         |   |   |  |
        [6] [4] [2] |
         |   |   +--|
         +---+---+
    """
    algorithms.append(make_algorithm(
        make_step(sel=ModulationSource.freg, fren=True),
        make_step(sel=ModulationSource.previous, mren=True),
        #
        make_step(sel=ModulationSource.none),
        make_step(sel=ModulationSource.previous, mren=True),
        #
        make_step(sel=ModulationSource.none),
        make_step(sel=ModulationSource.previous, mren=True),
    ))

    """Algorithm 7
                 |--|
                [1] |
                 |--|
        [5] [3] [2]
         |   |  /
        [6] [4]
         |   |
         +---+
    """
    algorithms.append(make_algorithm(
        make_step(sel=ModulationSource.freg, fren=True),
        make_step(sel=ModulationSource.previous, mren=True),
        make_step(sel=ModulationSource.none),
        make_step(sel=ModulationSource.mreg_plus_previous, mren=True),
        #
        make_step(sel=ModulationSource.none),
        make_step(sel=ModulationSource.previous, mren=True),
    ))

    """Algorithm 8
                  [1]
            |--|   |
        [5] | [3] [2]
         |  |--|  /
        [6]   [4]
         |     |
         +-----+
    """
    algorithms.append(make_algorithm(
        make_step(sel=ModulationSource.none),
        make_step(sel=ModulationSource.previous, mren=True),
        make_step(sel=ModulationSource.freg, fren=True),
        make_step(sel=ModulationSource.mreg_plus_previous, mren=True),
        #
        make_step(sel=ModulationSource.none),
        make_step(sel=ModulationSource.previous, mren=True),
    ))

    """Algorithm 9
                [1]
      |--|       |
      | [5] [3] [2]
      |--|   |  /
        [6] [4]
         |   |
         +---+
    """
    algorithms.append(make_algorithm(
        make_step(sel=ModulationSource.none),
        make_step(sel=ModulationSource.previous, mren=True),
        make_step(sel=ModulationSource.none),
        make_step(sel=ModulationSource.mreg_plus_previous, mren=True),
        #
        make_step(sel=ModulationSource.freg, fren=True),
        make_step(sel=ModulationSource.previous, mren=True),
    ))

    # TODO

    """Algorithm 32
                             |--|
        [6] [5] [4] [3] [2] [1] |
         |   |   |   |   |   +--|
         +---+---+---+---+---+
    """
    algorithms.append(make_algorithm(
        make_step(sel=ModulationSource.freg, fren=True, mren=True),
        make_step(sel=ModulationSource.none, mren=True),
        make_step(sel=ModulationSource.none, mren=True),
        make_step(sel=ModulationSource.none, mren=True),
        make_step(sel=ModulationSource.none, mren=True),
        make_step(sel=ModulationSource.none, mren=True),
    ))

    # Until all algorithms are implemented here, just duplicate Algorithm 32.
    while len(algorithms) < 32:
        algorithms.append(algorithms[-1])

    assert len(algorithms) <= 2**6
    return algorithms


def main():
    for algorithm_number, algorithm in enumerate(get_algorithms(), 1):
        print(f'// Algorithm {algorithm_number}')
        print(f'// ---------------------------------------------------')
        for step_number, step in enumerate(algorithm, 1):
            next_step = algorithm[step_number % len(algorithm)]

            address = ((algorithm_number - 1) << 3) | (step_number - 1)
            value = (
                (int(next_step.sel) << 2) |
                (int(step.mren) << 1) |
                (int(step.fren) << 0)
            )
            line = (
                f'@{address:04x}   {value:04x}    '
                f'// Operator {step_number} : '
                f'NEXT_OP_SEL={next_step.sel.name.upper()}'
            )
            if step.mren:
                line += ', MREN'
            if step.fren:
                line += ', FREN'
            print(line)
        print(f'// ---------------------------------------------------\n')


if __name__ == '__main__':
    sys.exit(main())
