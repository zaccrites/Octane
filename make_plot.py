
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
                MAX_VALUE = 2**(32 - 1)

                x.append(int(row[0]))
                samples.append(int(row[1]) / MAX_VALUE)
                # op1_output.append(int(row[2]) / MAX_VALUE)
                # op2_output.append(int(row[3]) / MAX_VALUE)


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


    print(min(samples), max(samples))



if __name__ == '__main__':
    sys.exit(main())
