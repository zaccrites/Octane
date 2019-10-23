
# Device

The target FPGA device is the Lattice [iCE40UP5K](http://www.latticesemi.com/en/Products/FPGAandCPLD/iCE40UltraPlus).


# Block RAM

* (8) Processed operator outputs for modulation and feedback
* (8) Raw operator outputs (with history) for filter DSP inputs
* (4) Filter coefficients configuration (symmetric FIR filters use each coefficient twice)
* (1) Phase accumulators
* (1) Phase step configuration
* (1) Envelope levels and states
* (4) Envelope configuration (Eight 8-bit parameters)
* (1) Feedback and waveform configuration
* (1) Algorithm instructions

**TOTAL: 29/30**


# Single-port RAM

I can save a lot of block RAM by using one of the four 16K x 16b SPRAMs
to store the sine wave table. There is no preload, so the microcontroller
will have to upload the data at boot.

The advantage is that I save seven or eight block RAMs, get 8x the depth
(16K vs 2K), and full resolution of 15 or 16 bits (instead of the 14 I get
when using only 7 block RAMs).

* (1) 16K-entry 16-bit quarter-wave sine lookup table ROM

**TOTAL: 1/4**


# DSP

* (8) Digital filter taps

**TOTAL: 8/8**

I'll still need to do multiplication in other areas,
but I'll convert those to repeated addition or use addition of
logarithms instead.


# PLL

There is a single PLL, which I will use to synthesize the real
`256 * 44.1 kHz = 11.3 MHz` (unless a faster sampling rate like 96 kHz is desired)
clock frequency.
