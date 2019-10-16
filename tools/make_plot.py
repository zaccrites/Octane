
import sys
import csv
from collections import namedtuple

import numpy as np
import matplotlib.pyplot as plt
from scipy.fftpack import fft


Record = namedtuple('Record', ['x', 'sample', 'expected'])

SAMPLE_FREQ = 44100
MAX_VALUE = 2**(16 - 1)


def main():

    if len(sys.argv) == 3:
        sample_start = int(sys.argv[1])
        sample_end = int(sys.argv[2])
    elif len(sys.argv) == 2:
        sample_start = 0
        sample_end = int(sys.argv[1])
    else:
        sample_start = 0
        sample_end = 1000

    data = []
    with open('data.csv', 'r') as f:
        reader = csv.reader(f)
        for i, row in enumerate(reader):
            data.append(Record(
                x=int(row[0]),
                sample=int(row[1]) / MAX_VALUE,
                expected=int(row[2]) / MAX_VALUE,
            ))

    def get_fft(samples):
        # https://stackoverflow.com/a/23378284
        sample_fft = fft(samples)
        d = len(sample_fft) // 2
        yf = abs(sample_fft[:(d-1)])

        k = np.arange(d - 1)
        T = len(yf) / SAMPLE_FREQ
        freq_labels = k / T

        # TODO: Clean this up. I had to add this line in order for the FFT
        # spikes at known frequencies to line up correctly.
        freq_labels = freq_labels / 2

        # Normalize output for relative frequency contributions to overall spectrum
        if max(yf) != 0:
            yf = yf / max(yf)

        return freq_labels, yf


    simulated_samples = [(d.x, d.sample) for d in data]
    expected_samples = [(d.x, d.expected) for d in data]
    difference_samples = [(d.x, d.expected - d.sample) for d in data]


    plots = [
        ('Simulated', simulated_samples),
    ]
    if 'expected' in sys.argv:
        plots.append(('Expected', expected_samples))
        plots.append(('Difference', difference_samples))

    fig, axes = plt.subplots(2, len(plots), figsize=(15, 10))
    for i, (name, samples) in enumerate(plots):
        sample_x, sample_y = list(zip(*samples))
        freq_labels, sample_fft = get_fft(sample_y)

        # TODO: Better way
        def get_index(row):
            return np.s_[row, i] if len(plots) > 1 else np.s_[row]

        axes[get_index(0)].set_title(f'Time Domain ({name})')
        axes[get_index(0)].plot(sample_x[sample_start:sample_end], sample_y[sample_start:sample_end])
        axes[get_index(0)].set_xlabel('Sample Number')
        axes[get_index(0)].set_ylabel('Amplitude')

        # TODO: Make this a command-line option for easier debugging
        axes[get_index(1)].set_title(f'Frequency Domain ({name})')
        axes[get_index(1)].set_xlabel('Frequency [Hz]')
        axes[get_index(1)].set_ylabel('Relative Strength')
        #
        use_linear_fft_plot = True
        if use_linear_fft_plot:
            import matplotlib.ticker as plticker
            axes[get_index(1)].plot(freq_labels, sample_fft, 'r')
            axes[get_index(1)].set_xlim([0, 33 * 100])
            axes[get_index(1)].xaxis.set_major_locator(plticker.MultipleLocator(base=200.0))
        else:
            axes[get_index(1)].semilogx(freq_labels, sample_fft, 'r')
            axes[get_index(1)].set_xlim([1, 20000])

        for ax in axes.flat:
            ax.grid(True, which='both', ls='-', color='0.65')

    plt.suptitle('Synthesizer Output')
    plt.savefig('data.png')

    print(f'Min: {min(d.sample for d in data):.04f}')
    print(f'Max: {max(d.sample for d in data):.04f}')

    # # peak frequency from FFT
    # if max(yf) != 0:
    #     peak_freq = max(zip(yf, freq_labels))
    #     print(f'{peak_freq[1]:.02f} Hz')


if __name__ == '__main__':
    sys.exit(main())
