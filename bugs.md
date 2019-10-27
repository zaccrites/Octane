
# Bugs

* Modulation with multiple (or even a single operator) doesn't seem to work right



# Features

* Re-enable all 32 voices
* Filtering
* Configuration should use an SPI interface
* Sine "ROM" data should be loaded at start, not configured with `$readmemh`



# Other

* Is curring off the sign bit like this in the sample_generator right?
  `r_CompensatedCarrierOutput <= {r_CompensatedCarrierOutputProduct[2][30:16], 1'b0};`
* The way configuration registers are written with respect to byte ordering
  vs. register numbers is a bit messy. It should be organized better.
* Use a real bug tracker?
