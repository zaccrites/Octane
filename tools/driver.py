
import sys
import argparse
import struct
import json
import math
from collections import namedtuple

import mido
import numpy as np
import simpleaudio as sa


OP_PARAM_PHASE_STEP =  0x00
OP_PARAM_ALGORITHM     =  0x01

OP_PARAM_ENVELOPE_ATTACK_LEVEL  =  0x02
OP_PARAM_ENVELOPE_SUSTAIN_LEVEL =  0x03
OP_PARAM_ENVELOPE_ATTACK_RATE   =  0x04
OP_PARAM_ENVELOPE_DECAY_RATE    =  0x05
OP_PARAM_ENVELOPE_RELEASE_RATE  =  0x06

OP_PARAM_FEEDBACK  =  0x07


PARAM_NOTEON_BANK0 =  0x10
PARAM_NOTEON_BANK1 =  0x11


Command = namedtuple('Command', ['register', 'value'])

def make_operator_command(voice_num, op_num, param, value):
    assert voice_num < 32
    assert op_num < 8
    assert param < 64
    assert value < 2**16
    register = (0b10 << 14) | (param << 8) | (op_num << 5) | voice_num
    return Command(register, value)



class Operator(object):

    def __init__(self, number, data):
        self.number = number
        self.data = data

    @property
    def is_carrier(self):
        return self.data['is_carrier']

    @property
    def algorithm_word(self):
        modulators = self.data['modulated_by']
        word = 0b000000  # First six bits are dontcares
        for op_num in [7, 6, 5, 4, 3, 2, 1]:
            word = (word << 1) | (1 if op_num in modulators else 0)
        word = (word << 1) | (1 if self.is_carrier else 0)
        assert word < 2**16
        return word

    def encode_algorithm(self, voice_num):
        yield make_operator_command(voice_num, self.number, OP_PARAM_ALGORITHM, self.algorithm_word)

    def encode_feedback(self, voice_num):
        yield make_operator_command(voice_num, self.number, OP_PARAM_FEEDBACK, self.data['feedback'])

    def encode_phase_step(self, voice_num, base_freq):
        real_freq = base_freq * self.data.get('frequency_ratio', 1.0)
        phase_step = frequency_to_phase_step(real_freq)
        yield make_operator_command(voice_num, self.number, OP_PARAM_PHASE_STEP, phase_step)

    def encode_levels(self, voice_num):
        # TODO
        yield make_operator_command(voice_num, self.number, OP_PARAM_ENVELOPE_ATTACK_LEVEL, 0xffff)
        yield make_operator_command(voice_num, self.number, OP_PARAM_ENVELOPE_SUSTAIN_LEVEL, 0xffff)
        yield make_operator_command(voice_num, self.number, OP_PARAM_ENVELOPE_ATTACK_RATE, 0xffff)
        yield make_operator_command(voice_num, self.number, OP_PARAM_ENVELOPE_DECAY_RATE, 0xffff)
        yield make_operator_command(voice_num, self.number, OP_PARAM_ENVELOPE_RELEASE_RATE, 0xffff)




class Patch(object):

    def __init__(self, data):
        self.operators = [Operator(i, op_data) for i, op_data in enumerate(data['operators'])]

    @classmethod
    def load(cls, path):
        with open(path, 'r') as f:
            return cls(json.load(f))

    def encode_config(self, voices=None):
        """Encode as a sequence of register write commands."""
        if voices is None:
            voices = range(32)

        for voice_num in voices:
            for operator in self.operators:
                yield from operator.encode_algorithm(voice_num)
                yield from operator.encode_feedback(voice_num)
                yield from operator.encode_levels(voice_num)

    def encode_note_change(self, voice_num, base_freq):
        for operator in self.operators:
            yield from operator.encode_phase_step(voice_num, base_freq)


def encode_sine_table_population():
    TABLE_SIZE = 16 * 1204
    BIT_DEPTH = 15
    MAX_RANGE = 1 << BIT_DEPTH
    MASK = MAX_RANGE - 1

    for i in range(TABLE_SIZE):
        # https://zipcpu.com/dsp/2017/08/26/quarterwave.html
        phase = (
            2 * math.pi *
            (2 * i + 1) /
            (2 * TABLE_SIZE * 4)
        )

        # https://stackoverflow.com/a/12946226
        sine = math.sin(phase)
        value = int(round(sine * MAX_RANGE)) & MASK

        register = (0b11 << 14) | i
        yield Command(register, value)



def encode_notes_on(voices):
    def encode_bank(bank_voices):
        value = 0
        for voice_on in bank_voices:
            value = (value << 1) | (1 if voice_on else 0)
        return value
    yield make_operator_command(0, 0, PARAM_NOTEON_BANK0, encode_bank(voices[0:16]))
    yield make_operator_command(0, 0, PARAM_NOTEON_BANK1, encode_bank(voices[16:32]))



def get_timestamped_messages(mid):
    t = 0
    for msg in mid:
        t += msg.time
        yield t, msg


def midi_note_number_to_frequency(note_number):
    # https://en.wikipedia.org/wiki/MIDI_tuning_standard#Frequency_values
    return 2.0**(note_number - 69) * 440


def frequency_to_phase_step(frequency):
    phase_step = 2**16 * frequency / 44100
    return int(round(phase_step))



def main():

    patch = Patch.load('patches/test2.json')

    commands = []
    def push_commands(t, cmds):
        commands.extend((t, cmd) for cmd in cmds)

    # Run these commands immediately
    push_commands(0.0, encode_sine_table_population())
    push_commands(0.0, patch.encode_config())


    voices_in_use = [None] * 32
    def take_voice(note_number):
        try:
            index = voices_in_use.index(None)
        except ValueError as exc:
            raise RuntimeError('Out of voices!') from exc
        else:
            voices_in_use[index] = note_number
            return index

    def free_voice(note_number):
        voice_num = voices_in_use.index(note_number)
        voices_in_use[voice_num] = None


    mid = mido.MidiFile(sys.argv[1])
    for t, msg in get_timestamped_messages(mid):
        if msg.type == 'note_on':
            # print(f'[t={t}] note {msg.note} ON')
            voice_num = take_voice(msg.note)
            freq = midi_note_number_to_frequency(msg.note)
            push_commands(t, patch.encode_note_change(voice_num, freq))
        elif msg.type == 'note_off':
            # print(f'[t={t}] note {msg.note} OFF')
            free_voice(msg.note)
        else:
            # Skip other messages
            continue

        active_voices = [note_number is not None for note_number in voices_in_use]
        push_commands(t, encode_notes_on(active_voices))


    with open('commands.bin', 'wb') as f:
        for t, cmd in commands:
            cmd_bytes = struct.pack('<fHH', t, cmd.register, cmd.value)
            f.write(cmd_bytes)

    print(f'Wrote {len(commands)} commands')



    # values = []
    # with open('data.csv', 'r') as f:
    #     for line in f:
    #         i, value = line.split(',')
    #         values.append(int(value))

    # data = np.asarray(values, dtype=np.uint16)
    # SAMPLE_RATE = 44100
    # NUM_CHANNELS = 1
    # BYTES_PER_SAMPLE = 2

    # play_obj = sa.play_buffer(data, NUM_CHANNELS, BYTES_PER_SAMPLE, SAMPLE_RATE)
    # play_obj.wait_done()


if __name__ == '__main__':
    sys.exit(main())
