
// Accumulate phase and combine with modulation and feedback phase.

`include "synth.svh"

// ===============================
// 2 clock cycles
// ===============================
module stage_phase_accumulation (
    input i_Clock,

    // ----------------------------------------------------------

    output logic unsigned [15:0] o_Phase,

    // ----------------------------------------------------------

    input VoiceOperatorID_t i_VoiceOperator,
    output VoiceOperatorID_t o_VoiceOperator,

    input AlgorithmWord_t i_AlgorithmWord,
    output AlgorithmWord_t o_AlgorithmWord,

    // ----------------------------------------------------------

    // configuration
    input logic [1:0] i_PhaseStepConfigWriteEnable,
    input VoiceOperatorID_t i_PhaseStepConfigWriteAddr,
    input logic unsigned [7:0] i_PhaseStepConfigWriteData

);


/// Accumulate phase unsigned (so that overflow produces the desired frequency).
/// When we add the signed modulation phase to this unsigned value (which
/// we make signed by prepending a 0 sign bit), we get a 17-bit signed phase
/// value which we can pass to the waveform generator.
logic unsigned [15:0] r_PhaseAccumulators [`NUM_VOICE_OPERATORS];
logic unsigned [15:0] r_PhaseStepConfig [`NUM_VOICE_OPERATORS];


// Pipeline registers
AlgorithmWord_t r_AlgorithmWord;
VoiceOperatorID_t r_VoiceOperator;
logic signed [15:0] r_AccumulatedPhase;
logic signed [15:0] r_PhaseStep;

logic signed [15:0] w_SteppedPhase;
always_comb begin
    w_SteppedPhase = r_AccumulatedPhase + r_PhaseStep;
end

always_ff @ (posedge i_Clock) begin

    // TODO: Make sure Lattice tools figure out to use write enable mask
    if (i_PhaseStepConfigWriteEnable[0])
        r_PhaseStepConfig[i_PhaseStepConfigWriteAddr][15:8] <= i_PhaseStepConfigWriteData;
    if (i_PhaseStepConfigWriteEnable[1])
        r_PhaseStepConfig[i_PhaseStepConfigWriteAddr][7:0] <= i_PhaseStepConfigWriteData;

    // Clock 1
    // ----------------------------------------------------------
    // Feeds back from Clock 2 to Clock 1
    r_PhaseAccumulators[r_VoiceOperator] <= w_SteppedPhase;

    r_AccumulatedPhase <= r_PhaseAccumulators[i_VoiceOperator];
    r_PhaseStep <= r_PhaseStepConfig[i_VoiceOperator];

    r_VoiceOperator <= i_VoiceOperator;
    r_AlgorithmWord <= i_AlgorithmWord;
    // ----------------------------------------------------------

    // Clock 2
    // ----------------------------------------------------------
    o_Phase <= w_SteppedPhase;

    o_VoiceOperator <= r_VoiceOperator;
    o_AlgorithmWord <= r_AlgorithmWord;
    // ----------------------------------------------------------

end


endmodule
