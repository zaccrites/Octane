
module synth (
    input logic i_Clock,
    input logic i_Reset,

    // TODO: Store in registers for individual operators and voices
    input logic unsigned [15:0] i_PhaseStep,

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

operator op (
    .i_Clock        (i_Clock),
    .i_Reset        (i_Reset),
    .i_PhaseStep    (i_PhaseStep),
    .i_EnvelopeLevel(w_EnvelopeLevel),
    .o_Sample       (o_Sample),
    .o_SampleReady (o_SampleReady)
);



always_ff @ (posedge i_Clock) begin

end


endmodule
