


module sine_function (
    input i_Clock,
    // input i_Reset,

    // TODO: Use both ports of the inferred BRAM

    input logic [12:0] i_Argument,

    output logic signed [17:0] o_Result

    // output o_DataValid
);

logic [17:0] r_Table[2047:0];


logic [10:0] r_Index;
logic [1:0] r_Negate;
// logic [1:0] r_DataValid;
logic [17:0] r_Value;



initial $readmemh("rtl/sine_function.hex", r_Table);



always_ff @ (posedge i_Clock) begin
    // if (i_Reset) begin
    //     // r_DataValid <= 2'b00;
    //     // o_DataValid <= 0;
    // end

    // The table gives the first quarter of the waveform, which we
    // can index directly using the lower 11 bits of the argument.
    // We can use the upper two bits and the sin function's symmetry
    // to synthesize the other three quarters.
    //
    //
    //     _--_
    //   /     \
    //  /       \
    //           \      /
    //            \_  _/
    //              ^^
    //
    //
    //
    // 00 - Index directly
    // 01 - Reverse argument
    // 10 - Negate result
    // 11 - Reverse argument and negate result
    //
    // We can then negate the result if the uppermost bit is set
    // (the negative half of the period), and reverse the argument
    // if the second highest bit is set.
    //

    // Stage 1: Compute Index and Negate Flag
    r_Negate[0] <= i_Argument[12];
    r_Index <= i_Argument[11] ? ~i_Argument[10:0] : i_Argument[10:0];

    // Stage 2: Look up value from table
    r_Negate[1] <= r_Negate[0];
    r_Value <= r_Table[r_Index];

    // Stage 3: Output (possibly negated) value
    o_Result <= r_Negate[1] ? ~r_Value : r_Value;


end


endmodule
