
// Accumulate phase and combine with modulation and feedback phase.

`include "synth.svh"

// ===============================
// 2 clock cycles
// ===============================
module stage_phase_accumulator (
    input i_Clock,

    // ----------------------------------------------------------

    output logic unsigned [15:0] o_Phase,

    // ----------------------------------------------------------

    input VoiceOperatorID_t i_VoiceOperator,
    output VoiceOperatorID_t o_VoiceOperator,

    // ----------------------------------------------------------

    // configuration
    input logic i_PhaseStepConfigWriteEnable,
    input VoiceOperatorID_t i_ConfigWriteAddr,
    input logic [15:0] i_ConfigWriteData

);


/// Accumulate phase unsigned (so that overflow produces the desired frequency).
/// When we add the signed modulation phase to this unsigned value (which
/// we make signed by prepending a 0 sign bit), we get a 17-bit signed phase
/// value which we can pass to the waveform generator.
logic unsigned [15:0] r_PhaseAccumulators [`NUM_VOICE_OPERATORS];
logic unsigned [15:0] r_PhaseStepConfig [`NUM_VOICE_OPERATORS];


// Pipeline registers
VoiceOperatorID_t r_VoiceOperator;
logic signed [15:0] r_AccumulatedPhase;
logic signed [15:0] r_PhaseStep;

logic signed [15:0] w_SteppedPhase;
always_comb begin
    w_SteppedPhase = r_AccumulatedPhase + r_PhaseStep;
end

always_ff @ (posedge i_Clock) begin

    // TODO: Make sure Lattice tools figure out to use write enable mask
    if (i_PhaseStepConfigWriteEnable)
        r_PhaseStepConfig[i_ConfigWriteAddr] <= i_ConfigWriteData;

    // Clock 1
    // ----------------------------------------------------------
    // Feeds back from Clock 2 to Clock 1
    r_PhaseAccumulators[r_VoiceOperator] <= w_SteppedPhase;

    r_AccumulatedPhase <= r_PhaseAccumulators[i_VoiceOperator];
    r_PhaseStep <= r_PhaseStepConfig[i_VoiceOperator];

    r_VoiceOperator <= i_VoiceOperator;
    // ----------------------------------------------------------

    // Clock 2
    // ----------------------------------------------------------
    o_Phase <= w_SteppedPhase;

    o_VoiceOperator <= r_VoiceOperator;
    // ----------------------------------------------------------

end


endmodule
