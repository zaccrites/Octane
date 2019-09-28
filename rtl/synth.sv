
module synth (
    input logic i_Clock,
    input logic i_Reset,

    // TODO: Store in registers for individual operators and voices
    // input logic unsigned [15:0] i_PhaseStep,

    // TODO: PWM, or output to DAC via SPI?
    output logic o_SampleReady,
    output logic signed [15:0] o_Sample
);



logic signed [15:0] w_EnvelopeLevel;
envelope_generator envelope (
    .i_Clock(i_Clock),
    .i_Reset(i_Reset),
    .o_Level(w_EnvelopeLevel)
);

logic signed [19:0] r_SampleBuffer;
logic signed [15:0] w_Subsample;
logic w_SampleReady;
logic w_SubsampleReady;
operator op (
    .i_Clock        (i_Clock),
    .i_Reset        (i_Reset),
    // .i_PhaseStep    (i_PhaseStep),
    .i_EnvelopeLevel(w_EnvelopeLevel),
    .o_Subsample       (w_Subsample),
    .o_SubsampleReady (w_SubsampleReady),
    .o_SampleReady    (w_SampleReady)
);

assign o_SampleReady = w_SampleReady;
assign o_Sample = r_SampleBuffer[19:4];

always_ff @ (posedge i_Clock) begin

    if (w_SampleReady)
        // If the last cycle emitted a sample, then clear the sample buffer.
        r_SampleBuffer <= 0;
    else if (w_SubsampleReady)
        // Accumulate 16 subsamples into a buffer, then emit the final sample.
        r_SampleBuffer <= r_SampleBuffer + {{4{w_Subsample[15]}}, w_Subsample};

    // Is this costing an extra clock? Does it matter?
    // It might be a clock behind the "latest" in the operator system,
    // but that doesn't matter.

end


endmodule
