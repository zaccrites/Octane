
# Device

The target FPGA device is the Lattice iCE40UP5K.


# Block RAM

This is my ideal arrangement:

* (7) 2048-entry 14-bit quarter-wave sine lookup table ROM
* (8) Operator outputs for modulation, including feedback
* (3) Raw operator outputs (with history) for filter DSP inputs
* (1) Algorithm instructions
* (1) Envelope levels and states
* (1) Phase accumulators
* (4) Envelope configuration (Eight 8-bit parameters)
* (3) Filter parameter configuration
* (1) Phase step configuration
* (1) Per-voice configuration
* (1) Feedback configuration
* (1) Waveform configuration

**TOTAL: 32/30**

This is too many, so here are some ideas to reduce utilization:

* Reduce length or width of the sine table ROM
* Combine feedback and waveform configuration: e.g. (feedback=6 bits, waveformType=3 bits, waveformParameter=7 bits)
* Use LUT RAM for per-voice configuration, since there are only 32 channels and the number
  of configuration bits is small.

Here is a modified version:

* (7) 2048-entry 14-bit quarter-wave sine lookup table ROM
* (8) Operator outputs for modulation, including feedback
* (3) Raw operator outputs (with history) for filter DSP inputs
* (1) Algorithm instructions
* (1) Envelope levels and states
* (1) Phase accumulators
* (4) Envelope configuration (Eight 8-bit parameters)
* (3) Filter parameter configuration
* (1) Phase step configuration
* (1) Feedback and waveform configuration

**TOTAL: 30/30**


# Single port RAM

There is a lot of single port RAM on this device, but without separate
read and write ports I'm not sure how to use it.

Possibly as a sample buffer of some kind?


# DSP

* (1) Multiple-modulator compensation
* (1) Carrier compensation
* (4) Operator output filter DSP
* (1) Operator output envelope
* (1) Operator feedback factor

**TOTAL: 8/8**

To make the filtering work I will need every tap I can possibly get.
I can potentially avoid some multiplication with carrier compensation
and multiple modulators by using `log` and `exp` sine tables,
though this complicates other waveforms.

Using a shifter for feedback like the DX7 did gets me one back
at least. Then I can at least have 5 without resorting to logarithms.

4 or 5 taps may be enough for what I want to do, actually.
-26 dB/decade (5% in stopband) is achievable at 44.1 kHz
with N=5.

The problem is going to be getting the block RAM.



# PLL

There is a single PLL, which I will use to synthesize the real
`256 * 44.1 kHz = 11.3 MHz` (unless a faster sampling rate like 96 kHz is desired)
clock frequency.
