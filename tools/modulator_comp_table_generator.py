
# Compensate for multiple modulators by dividing each output by
# the number of modulators (to avoid overflowing the phase variable).

# We pass in 7 bits indicating which operators will act as modulators.
# From this we count the number of bits and generate a number to use
# to divide by (multiply by reciprocal).


MAX_MODULATORS = 7

for i in range(2 ** MAX_MODULATORS):
    num_modulators = bin(i).count('1')
    compensation_factor = round(0x7fff / num_modulators) if num_modulators else 0
    print(f"7'b{i:07b}:  w_ModulationCompensationFactor = 16'h{compensation_factor:04x};  // {num_modulators} modulators")
