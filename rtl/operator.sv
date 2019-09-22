
`include "operator.svh"

module operator (
    input logic i_Clock,
    input logic i_Reset,

    input OperatorRegisters_t i_Registers,
    input logic signed [23:0] i_ModulatingPhase,

    output logic signed [23:0] o_Output
);


// TODO: Share a sine table between two operators.
// Perhaps the operators can come as pairs?

// TODO: Pre-adder with feedback amplitude

// TODO: There's a lot of multiplication going on in here.
// Can I share a multiplier to get e.g. two 9x12 bit multiplications
// out of a single DSP slice? Or three 6x8 bit multiplications?


logic signed [23:0] w_PrimaryPhase;
phase_generator phasegen (
    .i_Clock    (i_Clock),
    .i_Reset    (i_Reset),
    .i_Frequency(i_Registers.Frequency),
    .o_Phase    (w_PrimaryPhase)
);


// TODO: Bottom bits of phase not used
// verilator lint_off UNUSED
logic signed [23:0] w_CombinedPhase;
// verilator lint_on UNUSED
assign w_CombinedPhase = w_PrimaryPhase + i_ModulatingPhase;


logic signed [18:0] w_SinResult;
sine_function sin0 (
    .i_Clock   (i_Clock),
    .i_Argument(w_CombinedPhase[23:11]),
    .o_Result  (w_SinResult)
);


logic signed [15:0] r_AmplitudeFactor [1:0];


// TODO: Some bits of product not used?
// verilator lint_off UNUSED
logic signed [34:0] r_Product;
// verilator lint_on UNUSED

always_ff @ (posedge i_Clock) begin
    r_AmplitudeFactor[0] <= i_Registers.AmplitudeFactor;
    r_AmplitudeFactor[1] <= r_AmplitudeFactor[0];

    r_Product <= w_SinResult * r_AmplitudeFactor[1];
    o_Output <= r_Product[26:3];  // shift right by 8 to counter the 8 fractional bits
end


endmodule
