
// TODO

module envelope_generator (
    // verilator lint_off UNUSED
    input i_Clock,
    input i_Reset,
    // verilator lint_on UNUSED

    // Unsigned Q16 envelope level
    output logic signed [15:0] o_Level
);


// For now, output the full amplitude at all times
assign o_Level = 16'h7fff;


endmodule
