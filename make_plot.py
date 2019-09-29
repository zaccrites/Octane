
import sys
import csv

import numpy as np
import matplotlib.pyplot as plt
from scipy.fftpack import fft


def main():

    x = []
    samples = []

    SAMPLE_FREQ = 44100

    with open('data.csv', 'r') as f:
        reader = csv.reader(f)
        for i, row in enumerate(reader):
            MAX_VALUE = 2**(16 - 1)
            x.append(int(row[0]))
            samples.append(int(row[1]) / MAX_VALUE)

    # https://stackoverflow.com/a/23378284
    sample_fft = fft(samples)
    d = len(sample_fft) // 2
    yf = abs(sample_fft[:(d-1)])
    #
    k = np.arange(d - 1)
    T = len(yf) / SAMPLE_FREQ
    freq_labels = k / T

    # TODO: Clean this up. I had to add this line in order for the FFT
    # spikes at known frequencies to line up correctly.
    freq_labels = freq_labels / 2

    # Normalize output for relative freuqency contributions to overall spectrum
    if max(yf) != 0:
        yf = yf / max(yf)


    if len(sys.argv) == 3:
        sample_start = int(sys.argv[1])
        sample_end = int(sys.argv[2])
    elif len(sys.argv) == 2:
        sample_start = 0
        sample_end = int(sys.argv[1])
    else:
        sample_start = 0
        sample_end = 1000

    fig, axes = plt.subplots(2, figsize=(10, 10))
    #
    axes[0].set_title('Time Domain')
    axes[0].plot(x[sample_start:sample_end], samples[sample_start:sample_end])
    axes[0].set_xlabel('Sample Number')
    axes[0].set_ylabel('Amplitude')
    #
    axes[1].set_title('Frequency Domain')
    axes[1].semilogx(freq_labels, yf, 'r')
    axes[1].set_xlim([1, 20000])
    axes[1].set_xlabel('Frequency [Hz]')
    axes[1].set_ylabel('Relative Strength')
    #
    for ax in axes:
        ax.grid(True, which='both', ls='-', color='0.65')
    #
    plt.suptitle('Synthesizer Output')
    plt.savefig('data.png')

    print(f'Min: {min(samples):.04f}')
    print(f'Max: {max(samples):.04f}')

    # peak frequency from FFT
    if max(yf) != 0:
        peak_freq = max(zip(yf, freq_labels))
        print(f'{peak_freq[1]:.02f} Hz')


if __name__ == '__main__':
    sys.exit(main())
