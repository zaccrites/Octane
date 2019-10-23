
`include "synth.sv"

module stage_waveform_generation (
    input i_Clock

    // For sine waves, we can ignore the sign bit completely
    // (though the other inverted bits in a two's complement representation
    // of the negative phase are still important).
    //
    // Since the table is only 2048 entries, we ignore some of the other
    // bits too.
    //
    // verilator lint_off UNUSED
    input logic signed [16:0] i_Phase,
    // verilator lint_on UNUSED

    output logic signed [15:0] o_Waveform
);


initial $readmemh("roms/sine_rom.hex", r_SineTable);
logic unsigned [13:0] r_SineTable[2048];
logic unsigned [10:0] r_SineTableIndex;


/// Negate the output in the second half of the waveform
logic w_NegateOutput;
/// Negate the phase index to the table in the second and fourth quadrant.
logic w_NegatePhase;
/// These are the actual (possibly inverted) phase index bits.
logic unsigned [10:0] w_RawPhaseArgument;

always_comb begin
    // <sign> = i_Phase[16];
    // <ignored> = i_Phase[15];
    w_NegateOutput = i_Phase[14];
    w_NegatePhase = i_Phase[13];
    w_RawPhaseArgument = i_Phase[12:2];
end


logic r_NegateOutput [?];
logic unsigned [12:0] r_PhaseArgument;
logic unsigned [13:0] r_RawQuarterWaveSine;

logic unsigned [15:0] w_QuarterWaveSine;
assign w_QuarterWaveSine = {2'b00, r_RawQuarterWaveSine};

always_ff @ (posedge i_Clock) begin

    // Clock 1
    // ----------------------------------------------------------
    r_NegateOutput[0] <= w_NegateOutput;
    r_PhaseArgument <= w_NegatePhase ? ~w_RawPhaseArgument : w_RawPhaseArgument;
    // ----------------------------------------------------------

    // Clock 2
    // ----------------------------------------------------------
    r_NegateOutput[1] <= r_NegateOutput[0];
    r_RawQuarterWaveSine <= r_SineTable[r_PhaseArgument];
    // ----------------------------------------------------------

    // Clock 3
    // ----------------------------------------------------------
    o_Waveform <= r_NegateOutput[1] ? ~w_QuarterWaveSine : w_QuarterWaveSine;
    // ----------------------------------------------------------

end


endmodule
