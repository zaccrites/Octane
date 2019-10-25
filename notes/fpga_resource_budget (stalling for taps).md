


This is a WIP change meant to find extra memory and DSPs for filter taps,
assuming that we stall the pipeline every cycle in order to read
extra memory out of SPRAM and re-use DSP elements.



# Device

The target FPGA device is the Lattice [iCE40UP5K](http://www.latticesemi.com/en/Products/FPGAandCPLD/iCE40UltraPlus).


# Block RAM

* (8) Processed operator outputs for modulation and feedback
* (12) Raw operator outputs (with history) for filter DSP inputs
* (1) Phase accumulators
* (1) Envelope levels and states
* (1) Feedback and waveform configuration


* (1) Phase step configuration
* (1) Algorithm instructions

**TOTAL: 30/30**


# Single-port RAM

I can save a lot of block RAM by using one of the four 16K x 16b SPRAMs
to store the sine wave table. There is no preload, so the microcontroller
will have to upload the data at boot.

The advantage is that I save seven or eight block RAMs, get 8x the depth
(16K vs 2K), and full resolution of 15 or 16 bits (instead of the 14 I get
when using only 7 block RAMs).

I can also save block RAM by using SPRAM for configuration tables which are
unlikely to be e.g. modulated by an LFO at runtime, and are like to be
written only during initial patch configuration or when a key is pressed
or released.

The envelope configuration is also especially good for this because the
SPRAM is long enough to store all of the parameters. However since I have to
read four 16-bit words, it will take four clock cycles to read the whole
thing. Therefore I wonder if I should go even further and do three stalls
per cycle (4x the frequency). Then I can store some of this configuration
information in the long SPRAM, get more filter taps, and still come in under
100 MHz, even oversampling at 96 kHz (`96 kHz * 256 * 4 = 98.304 MHz`).

* (1) 16K-entry 16-bit quarter-wave sine lookup table ROM
* (1)
    * Algorithm instructions (8 bits)
    * Filter coefficients configuration (16 bits)
    * Feedback/waveform configuration (16 bits)


    * Envelope configuration (64 bits: eight 8-bit parameters))


**TOTAL: 1/4**


# DSP

* (8) Digital filter taps

**TOTAL: 8/8**

I'll still need to do multiplication in other areas,
but I'll convert those to repeated addition or use addition of
logarithms instead.

Although if I have to stall the pipeline to do extra filtering anyway,
then can I use them like this?

* Phase 1
    1. Envelope attenuation
    2. Feedback factor
    3. Carrier compensation
    4.
    5. Filter
    6. Filter
    7. Filter
    8. Filter



# PLL

There is a single PLL, which I will use to synthesize the real
`256 * 44.1 kHz = 11.3 MHz` (unless a faster sampling rate like 96 kHz is desired)
clock frequency.
