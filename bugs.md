
# Bugs

* Modulation with multiple (or even a single operator) doesn't seem to work right



# Features

* Only update operator level after a zero crossing to avoid waveform artifacts
  from the sudden jump in amplitude.
* Filtering
* Configuration should use an SPI interface
* Sine "ROM" data should be loaded at start, not configured with `$readmemh`



# Other

* Is curring off the sign bit like this in the sample generator and atteunator actually correct?
  `r_CompensatedCarrierOutput <= {r_CompensatedCarrierOutputProduct[2][30:16], 1'b0};`
  `o_Waveform <= {r_AttenuatedWaveformProduct[2][30:16], 1'b0};`
* The way configuration registers are written with respect to byte ordering
  vs. register numbers is a bit messy. It should be organized better.
* Use a real bug tracker?