
`include "synth.svh"

module stage_waveform_generator (
    input i_Clock,

    input `VOICE_OPERATOR_ID i_VoiceOperator,
    output `VOICE_OPERATOR_ID o_VoiceOperator,

    input logic `ALGORITHM_WORD i_AlgorithmWord,
    output logic `ALGORITHM_WORD o_AlgorithmWord,

    input logic i_NoteOn,
    output logic o_NoteOn,

    input logic i_SineTableWriteEnable,
    input logic [13:0] i_SineTableWriteAddress,
    input logic [15:0] i_SineTableWriteValue,

    // For sine waves, we ignore the sign bit completely
    // (though the other inverted bits in a two's complement representation
    // of the negative phase are still important).
    //
    // verilator lint_off UNUSED
    input logic signed [16:0] i_Phase,
    // verilator lint_on UNUSED

    output logic [15:0] o_SINE_TABLE_OUTPUT,

    output logic signed [15:0] o_Waveform
);


// The most significant bit isn't used, since we synthesize it from phase.
// verilator lint_off UNUSED
logic [15:0] w_SineTableOutput;
// verilator lint_on UNUSED


// Write address takes priority when writing (SPRAM has a shared read/write port)
logic [13:0] w_SineTableIndex;
assign w_SineTableIndex = i_SineTableWriteEnable ? i_SineTableWriteAddress : r_PhaseArgument;


`ifdef YOSYS

logic [3:0] w_MaskWREN;
assign w_MaskWREN = i_SineTableWriteEnable ? 4'b1111 : 4'b0000;

SB_SPRAM256KA sine_spram (
    .CLOCK(i_Clock),
    .ADDRESS(w_SineTableIndex),
    .DATAIN(i_SineTableWriteValue),
    .MASKWREN(w_MaskWREN),
    .WREN(i_SineTableWriteEnable),
    .CHIPSELECT(1'b1),
    .STANDBY(1'b0),
    .POWEROFF(1'b1),
    .SLEEP(1'b0),
    .DATAOUT(w_SineTableOutput)
);

`else

logic [15:0] r_SineTable[16 * 1024];
assign w_SineTableOutput = r_SineTable[r_PhaseArgument];

always_ff @ (posedge i_Clock) begin
    if (i_SineTableWriteEnable)
        r_SineTable[w_SineTableIndex] <= i_SineTableWriteValue;
end

`endif


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
    r_RawQuarterWaveSine <= w_SineTableOutput[14:0];

    r_VoiceOperator[1] <= r_VoiceOperator[0];
    r_AlgorithmWord[1] <= r_AlgorithmWord[0];
    r_NoteOn[1] <= r_NoteOn[0];
    // ----------------------------------------------------------

    // Clock 3
    // ----------------------------------------------------------
    o_Waveform <= r_NegateOutput[1] ? ~w_QuarterWaveSine : w_QuarterWaveSine;
    o_SINE_TABLE_OUTPUT <= w_QuarterWaveSine;

    o_VoiceOperator <= r_VoiceOperator[1];
    o_AlgorithmWord <= r_AlgorithmWord[1];
    o_NoteOn <= r_NoteOn[1];

    // ----------------------------------------------------------

end


endmodule
