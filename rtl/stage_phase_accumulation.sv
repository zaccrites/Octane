
// Accumulate phase and combine with modulation and feedback phase.

`include "synth.svh"

// ===============================
// 2 clock cycles
// ===============================
module stage_phase_accumulation (
    input i_Clock,

    // ----------------------------------------------------------

    input signed [15:0] i_ModulationPhase,
    output signed [15:0] o_ModulatedPhase,

    // ----------------------------------------------------------

    input VoiceOperatorID_t i_VoiceOperator,
    output VoiceOperatorID_t o_VoiceOperator,

    input AlgorithmWord_t i_Algorithm,
    output AlgorithmWord_t o_Algorithm,

    // ----------------------------------------------------------

    // configuration
    input logic i_PhaseStepConfigWriteEnable,
    input VoiceOperatorID_t i_PhaseStepConfigWriteAddr,
    input logic signed [15:0] i_PhaseStepConfigWriteData

);


/// Accumulate phase unsigned (so that overflow produces the desired frequency).
/// When we add the signed modulation phase to this unsigned value (which
/// we make signed by prepending a 0 sign bit), we get a 17-bit signed phase
/// value which we can pass to the waveform generator.
logic unsigned [15:0] r_PhaseAccumulators [NUM_VOICE_OPERATORS];
logic unsigned [15:0] r_PhaseStepConfig [NUM_VOICE_OPERATORS];


// Pipeline registers
AlgorithmWord_t r_Algorithm;
VoiceOperatorID_t r_VoiceOperator;
logic signed [15:0] r_AccumulatedPhase;
logic signed [15:0] r_PhaseStep;
logic signed [15:0] r_ModulationPhase;


logic signed [15:0] w_SteppedPhase;
always_comb begin
    w_SteppedPhase = r_AccumulatedPhase + r_PhaseStep;
end

always_ff @ (posedge i_Clock) begin

    if (i_PhaseStepConfigWriteEnable)
        r_PhaseStepConfig[i_PhaseStepConfigWriteAddr] <= i_PhaseStepConfigWriteData;

    // Clock 1
    // ----------------------------------------------------------

    r_PhaseAccumulators[r_VoiceOperator] <= w_SteppedPhase;

    r_AccumulatedPhase <= r_PhaseAccumulators[i_VoiceOperator];
    r_PhaseStep <= r_PhaseStepConfig[i_VoiceOperator];
    r_ModulationPhase <= i_ModulationPhase;

    r_VoiceOperator <= i_VoiceOperator;
    r_Algorithm <= i_Algorithm;

    // ----------------------------------------------------------

    // Clock 2
    // ----------------------------------------------------------

    // TODO: Feedback
    o_ModulatedPhase <= w_SteppedPhase + r_ModulationPhase;

    o_VoiceOperator <= r_VoiceOperator;
    o_Algorithm <= r_Algorithm;

    // ----------------------------------------------------------

end


endmodule
