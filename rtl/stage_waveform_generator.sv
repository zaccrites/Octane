
`include "synth.svh"

module stage_waveform_generator (
    input i_Clock,

    input `VOICE_OPERATOR_ID i_VoiceOperator,
    output `VOICE_OPERATOR_ID o_VoiceOperator,

    input `ALGORITHM_WORD i_AlgorithmWord,
    output `ALGORITHM_WORD o_AlgorithmWord,

    input logic i_NoteOn,
    output logic o_NoteOn,

    // For sine waves, we ignore the sign bit completely
    // (though the other inverted bits in a two's complement representation
    // of the negative phase are still important).
    //
    // verilator lint_off UNUSED
    input logic signed [16:0] i_Phase,
    // verilator lint_on UNUSED

    output logic signed [15:0] o_Waveform
);


// TODO: Make this a RAM written externally at boot.
initial $readmemh("roms/sine_rom.hex", r_SineTable);
logic [14:0] r_SineTable[16 * 1024];


logic `VOICE_OPERATOR_ID r_VoiceOperator [2];
logic `ALGORITHM_WORD r_AlgorithmWord [2];
logic r_NoteOn [2];


/// Negate the output in the second half of the waveform
logic w_NegateOutput;
/// Negate the phase index to the table in the second and fourth quadrant.
logic w_NegatePhase;
/// These are the actual (possibly inverted) phase index bits.
logic [13:0] w_RawPhaseArgument;

always_comb begin
    // <sign, ignored> = i_Phase[16];
    w_NegateOutput = i_Phase[15];
    w_NegatePhase = i_Phase[14];
    w_RawPhaseArgument = i_Phase[13:0];
end


logic r_NegateOutput [2];
logic [13:0] r_PhaseArgument;
logic [14:0] r_RawQuarterWaveSine;
logic [15:0] w_QuarterWaveSine;
assign w_QuarterWaveSine = {1'b0, r_RawQuarterWaveSine};

always_ff @ (posedge i_Clock) begin

    // Clock 1
    // ----------------------------------------------------------
    r_NegateOutput[0] <= w_NegateOutput;
    r_PhaseArgument <= w_NegatePhase ? ~w_RawPhaseArgument : w_RawPhaseArgument;

    r_VoiceOperator[0] <= i_VoiceOperator;
    r_AlgorithmWord[0] <= i_AlgorithmWord;
    r_NoteOn[0] <= i_NoteOn;
    // ----------------------------------------------------------

    // Clock 2
    // ----------------------------------------------------------
    r_NegateOutput[1] <= r_NegateOutput[0];
    r_RawQuarterWaveSine <= r_SineTable[r_PhaseArgument];

    r_VoiceOperator[1] <= r_VoiceOperator[0];
    r_AlgorithmWord[1] <= r_AlgorithmWord[0];
    r_NoteOn[1] <= r_NoteOn[0];
    // ----------------------------------------------------------

    // Clock 3
    // ----------------------------------------------------------
    o_Waveform <= r_NegateOutput[1] ? ~w_QuarterWaveSine : w_QuarterWaveSine;

    o_VoiceOperator <= r_VoiceOperator[1];
    o_AlgorithmWord <= r_AlgorithmWord[1];
    o_NoteOn <= r_NoteOn[1];
    // ----------------------------------------------------------

end


endmodule
