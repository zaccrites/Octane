
// The iCE40UP5K has 30 4kb RAM blocks.
// We can use 8 of them to create a 2048-wide 16b sine table.
// Technically we can synthesize a 17th bit from symmetry of the sine function,
// but the DSP elements can only do a 16x16 bit multiply (unless cascaded).

// The table gives the first quarter of the waveform, which we
// can index directly using the lower 11 bits of the phase.
// We can use the upper two bits and the sin function's symmetry
// to synthesize the other three quarters.
//
// 00 - Index directly
// 01 - Reverse phase
// 10 - Negate result
// 11 - Reverse phase and negate result
//
// We can then negate the result if the uppermost bit is set
// (the negative half of the period), and reverse the phase
// if the second highest bit is set.
//

module sine_function (
    input i_Clock,
    input logic [12:0] i_Phase,
    output logic signed [15:0] o_Result
);

// TODO: Can we use the 16th bit of the table here?
// We'd end up with 17 bits, which I'm not sure is useful.
// Alternatively, can we step down to a 14 bit table and
// synthesize a 15th bit to save some block RAM? Do we need to?
logic [14:0] r_Table[2047:0];

logic [10:0] r_Index;
logic [1:0] r_Negate;
logic [15:0] r_Result;

initial $readmemh("rtl/sine_function.hex", r_Table);

always_ff @ (posedge i_Clock) begin
    // Stage 1: Compute Index and Negate Flag
    r_Negate[0] <= i_Phase[12];
    r_Index <= i_Phase[11] ? ~i_Phase[10:0] : i_Phase[10:0];

    // Stage 2: Look up value from table
    r_Negate[1] <= r_Negate[0];
    r_Result <= {1'b0, r_Table[r_Index]};

    // Stage 3: Output (possibly negated) value
    o_Result <= r_Negate[1] ? ~r_Result : r_Result;
end

endmodule
