
import sys
import time
import socket
import struct

HOST = 'localhost'
PORT = 5001


# Note that python-rtmidi can be used to communicate directly over
# a MIDI port if desired (instead of a network interface)
import mido

# For Pokemon Gold MIDI music files:
# https://www.khinsider.com/midi/gameboy/pokemon-gold
mid = mido.MidiFile('opening.mid')
for i, midi_msg in enumerate(mid.play()):
    # import pdb; pdb.set_trace()
    print(i, midi_msg)
sys.exit(0)

# Instead of an interactive keyboard, I guess just send messages and have
# the simulator prepare a complete song.

# Can either parse MIDI files for existing songs to replicate,
# or send custom sequences to test e.g. operator combinations
# with values as compared to KQ Dixie





def set_registers(assignments):
    commands = []
    for reg_number, value in assignments:
        command_bytes = struct.pack('!HH', reg_number, value)
        commands.append(command_bytes)
    return b''.join(commands)


def to_fixed(value: float) -> int:
    return int(value * 0x7fff)


def carriers_for_algorithm(algorithm: int) -> int:
    if algorithm in {16, 17, 18}:
        return 1
    elif algorithm in {1, 2, 3, 4, 7, 8, 9, 10, 11, 12, 13, 14, 15}:
        return 2
    elif algorithm in {5, 6, 19, 20, 26, 27, 28}:
        return 3
    elif algorithm in {21, 22, 23, 29, 30}:
        return 4
    elif algorithm in {24, 25, 31}:
        return 5
    elif algorithm == 32:
        return 6
    else:
        raise ValueError(algorithm)


SAMPLE_FREQUENCY = 44100

def calc_phase_step(frequency: float) -> int:
    # Formula:
    # phaseStep = 2^N * f / FS
    # where N is the number of bits of phase accumulation
    # FS is the sample frequency
    # and f is the desired tone frequency
    return int((2 ** 16) * frequency / SAMPLE_FREQUENCY)




VOICE_PARAM_KEYON =  0x00
VOICE_PARAM_ALGORITHM =  0x01
VOICE_PARAM_AMPLITUDE_ADJUST = 0x02

OP_PARAM_PHASE_STEP = 0x00
OP_PARAM_WAVEFORM = 0x01
OP_PARAM_ATTACK_LEVEL = 0x03
OP_PARAM_SUSTAIN_LEVEL = 0x04
OP_PARAM_ATTACK_RATE = 0x05
OP_PARAM_DECAY_RATE = 0x06
OP_PARAM_RELEASE_RATE = 0x07

OP_WAVEFORM_SINE = 0x0000
OP_WAVEFORM_SQUARE = 0x0001


def voice_reg_num(voice_num, param):
    return voice_op_reg_num(voice_num, 0, param)

def voice_op_reg_num(voice_num, op_num, param):
    return (
        ((voice_num & 0x001f) << 11) |  # 5 bit voice
        ((op_num & 0x0007) << 8) |      # 3 bit operator
        (param & 0x00ff)                # 8 bit parameter
    )


# def set_envelope(attack_level, sustain_level)


def main():
    commands = []

    for voice_num in range(1, 16+1):
        keyon = voice_num == 1
        algorithm = 1

        for op_num in range(1, 6+1):

            if voice_num == 1:
                if op_num == 5:
                    phase_step = calc_phase_step(220.0)
                    #
                    attack_level = 1.0
                    sustain_level = 0.2
                    attack_rate = 0.001
                    decay_rate = 0.05
                    release_rate = 0.005

                elif op_num == 6:
                    phase_step = calc_phase_step(440.0)
                    #
                    attack_level = 1.0
                    sustain_level = 0.7
                    attack_rate = 0.05
                    decay_rate = 0.0005
                    release_rate = 0.005

                else:
                    phase_step = calc_phase_step(1000.0)
                    #
                    attack_level = 0.0
                    sustain_level = 0.0
                    attack_rate = 0.0
                    decay_rate = 0.0
                    release_rate = 0.0
            else:
                phase_step = calc_phase_step(1000.0)
                #
                attack_level = 0.0
                sustain_level = 0.0
                attack_rate = 0.0
                decay_rate = 0.0
                release_rate = 0.0


            commands.extend([
                (voice_op_reg_num(voice_num, op_num, OP_PARAM_WAVEFORM), OP_WAVEFORM_SINE),
                (voice_op_reg_num(voice_num, op_num, OP_PARAM_PHASE_STEP), phase_step),
                #
                (voice_op_reg_num(voice_num, op_num, OP_PARAM_ATTACK_LEVEL), to_fixed(attack_level)),
                (voice_op_reg_num(voice_num, op_num, OP_PARAM_SUSTAIN_LEVEL), to_fixed(sustain_level)),
                (voice_op_reg_num(voice_num, op_num, OP_PARAM_ATTACK_RATE), to_fixed(attack_rate)),
                (voice_op_reg_num(voice_num, op_num, OP_PARAM_DECAY_RATE), to_fixed(decay_rate)),
                (voice_op_reg_num(voice_num, op_num, OP_PARAM_RELEASE_RATE), to_fixed(release_rate)),
            ])

        commands.extend([
            (voice_reg_num(voice_num, VOICE_PARAM_ALGORITHM), algorithm - 1),
            (voice_reg_num(voice_num, VOICE_PARAM_AMPLITUDE_ADJUST), to_fixed(carriers_for_algorithm(algorithm))),
            (voice_reg_num(voice_num, VOICE_PARAM_KEYON), keyon),
        ])

    # TODO: Clean up these function and variable names
    print(f'{len(commands)} commands', file=sys.stderr)
    message = set_registers(commands)
    print(f'  message has {len(message)} bytes', file=sys.stderr)


    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = (HOST, PORT)
    print('connecting to %s on port %s' % server_address, file=sys.stderr)
    sock.connect(server_address)

    sock.sendall(message)

    data = sock.recv(3)
    print(f'got back: {data}', file=sys.stderr)


    # Wait, then send signal to terminate
    time.sleep(3.000)
    print('Sending message to quit...')
    message = set_registers([(0xffff, 0xffff)])
    sock.sendall(message)
    data = sock.recv(3)
    print(f'got back: {data}', file=sys.stderr)




if __name__ == '__main__':
    sys.exit(main())
