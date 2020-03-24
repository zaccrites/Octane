
`include "synth.svh"


module stage_phase_accumulator (
    input i_Clock,

    output logic [15:0] o_Phase,

    input logic `VOICE_OPERATOR_ID i_VoiceOperator,
    output logic `VOICE_OPERATOR_ID o_VoiceOperator,

    output logic o_NoteOn,

    input logic i_NoteOnConfigWriteEnable,
    input logic i_PhaseStepConfigWriteEnable,
    input logic `VOICE_OPERATOR_ID i_ConfigWriteAddr,
    input logic [15:0] i_ConfigWriteData
);


/// Accumulate phase unsigned (so that overflow produces the desired frequency).
/// When we add the signed modulation phase to this unsigned value (which
/// we make signed by prepending a 0 sign bit), we get a 17-bit signed phase
/// value which we can pass to the waveform generator.
logic [15:0] r_PhaseAccumulators [`NUM_VOICE_OPERATORS];
logic [15:0] r_PhaseStepConfig [`NUM_VOICE_OPERATORS];

/// Stored per-voice
logic [`NUM_VOICES-1:0] r_NoteOnConfig;


// Pipeline registers
logic `VOICE_OPERATOR_ID r_VoiceOperator;
logic signed [15:0] r_AccumulatedPhase;
logic signed [15:0] r_PhaseStep;
logic r_NoteOn;

logic signed [15:0] w_SteppedPhase;
always_comb begin
    // w_SteppedPhase = r_NoteOn ? (r_AccumulatedPhase + r_PhaseStep) : 0;
    w_SteppedPhase = r_AccumulatedPhase + r_PhaseStep;
end

always_ff @ (posedge i_Clock) begin

    if (i_PhaseStepConfigWriteEnable)
        r_PhaseStepConfig[i_ConfigWriteAddr] <= i_ConfigWriteData;

    if (i_NoteOnConfigWriteEnable)
        r_NoteOnConfig[`NUM_VOICES-1:0] <= i_ConfigWriteData[`NUM_VOICES-1:0];

    // Clock 1
    // ----------------------------------------------------------
    // Feeds back from Clock 2 to Clock 1
    r_PhaseAccumulators[r_VoiceOperator] <= w_SteppedPhase;

    r_AccumulatedPhase <= r_PhaseAccumulators[i_VoiceOperator];
    r_PhaseStep <= r_PhaseStepConfig[i_VoiceOperator];
    r_NoteOn <= r_NoteOnConfig[getVoiceID(i_VoiceOperator)];

    r_VoiceOperator <= i_VoiceOperator;
    // ----------------------------------------------------------

    // Clock 2
    // ----------------------------------------------------------
    o_Phase <= w_SteppedPhase;
    o_NoteOn <= r_NoteOn;
    o_VoiceOperator <= r_VoiceOperator;
    // ----------------------------------------------------------

end


endmodule
