
`ifndef OPERATOR_SVH
`define OPERATOR_SVH

typedef struct packed
{
    // TODO: These factors probably don't need to be 16 bits.

    // 8 integer bits, 8 fractional bits
    logic signed [15:0] AmplitudeFactor;  // TODO: ADSR
    // logic signed [15:0] FeedbackFactor;  // TODO: Feedback

    // TODO: Alternative waveforms

    logic unsigned [23:0] Frequency;

    // Options for ADSR envelope generation:
    //
    // 1) e.g. attack rate given in time to reach maximum
    // 2) e.g. attack rate given in amount increase per unit time
    //
    // Option 2 has some merit, as no division is necessary.
    // Just add the attack amount to the accumulator until the peak
    // is reached. Then subtract the decay amount until sustain
    // is reached. Then hold until key is released, and begin subtracting
    // amount until reaching zero.
    //
    // That means that the higher the attack rate, the faster it is.
    // Same with the others.

    // FUTURE: I noticed a bug at one point where the multiplier had to be increased
    // to 256 to get back to the normal unity amplitude. I thought that this could
    // possibly be made into a feature, since e.g. ADSR envelopes will only ever
    // want to lower the volume (I think), and this would give more precision to
    // do that than 256 steps only (plus I think the use case to increase above
    // unity will be rather limited). It's worth considering for the future.
    //
    // Also remember that the number will potentially have to be adjusted to
    // for multiple mixed voices all adding up to full output signal.


} OperatorRegisters_t;

`endif
