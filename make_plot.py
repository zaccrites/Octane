
import sys
import csv

import matplotlib.pyplot as plt


def main():

    series_names = None
    x = []
    samples = []
    # op1_output = []
    # op2_output = []


    with open('data.csv', 'r') as f:
        reader = csv.reader(f)
        for i, row in enumerate(reader):
            if i == 0:
                series_names = row
            else:
                MAX_VALUE = 2**(16 - 1)
                SAMPLE_FREQ = 44100

                # x.append(int(row[0]) / SAMPLE_FREQ)
                x.append(int(row[0]))
                samples.append(int(row[1]) / MAX_VALUE)

                # op1_output.append(int(row[2]) / MAX_VALUE)
                # op2_output.append(int(row[3]) / MAX_VALUE)


    # This is extremely crude. It would be cool to do a Fourier transform
    # on the output instead to get the frequency components
    # (especially to debug operator combinations)
    # Having this output on the real keyboard would be cool too.
    zeroes = []
    for i, (xi, samplei) in enumerate(zip(x, samples)):
        if i > 0 and samples[i - 1] < 0 and samplei > 0:
            zeroes.append((xi / SAMPLE_FREQ, samplei))
    if zeroes:
        x_range = max([zero[0] for zero in zeroes]) - min([zero[0] for zero in zeroes])
        # y_range = max([zero[1] for zero in zeroes]) - min([zero[1] for zero in zeroes])
        print(f'VERY approximate frequency is {len(zeroes) / x_range} Hz')
        print(f'Num zeroes: {len(zeroes)}')
    else:
        print('No zeroes found')


    # plt.plot(x, samples, x, op1_output, x, op2_output)

    # https://matplotlib.org/3.1.0/gallery/subplots_axes_and_figures/subplots_demo.html
    # fig, axes = plt.subplots(2)
    # axes[0].set_title('Output Sample')
    # axes[0].plot(x, samples)
    # # axes[1].set_title('Operator 1 Output')
    # # axes[1].plot(x, op1_output, 'tab:orange')
    # # axes[2].set_title('Operator 2 Output')
    # # axes[2].plot(x, op2_output, 'tab:green')

    # for ax in axes:
    #     ax.label_outer()
    #     ax.grid(True)

    fig = plt.figure()
    plt.plot(x, samples)
    # plt.suptitle('Output')
    plt.grid(True)
    plt.savefig('data.png')


    print(f'Min: {min(samples):.04f}')
    print(f'Max: {max(samples):.04f}')



if __name__ == '__main__':
    sys.exit(main())
