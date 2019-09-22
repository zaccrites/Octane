
`include "registers.svh"

module synth (
    input i_Clock,
    input i_Reset,



    // Frequency in radians per second
    input logic [23:0] i_Frequency1,
    input logic [23:0] i_Frequency2,

    input logic [15:0] i_Amp1,
    input logic [15:0] i_Amp2,





    // TODO: Use PWM instead of outputting the full sample
    output [23:0] o_Sample,


    // Sign extend in Verilog because it's easier than C++ :)
    output [31:0] o_Sample32,


    output [31:0] o_Sample32_1,
    output [31:0] o_Sample32_2

);


assign o_Sample32 = {{8{o_Sample[23]}}, o_Sample};


assign o_Sample32_1 = {{8{w_Operator1Output[23]}}, w_Operator1Output};
assign o_Sample32_2 = {{8{w_Operator2Output[23]}}, w_Operator2Output};




// TODO: This may be cumbersome for multiple voices.
// Although each voice can likely be its own module anyway.
//
logic [23:0] w_Operator1Phase;
logic [23:0] w_Operator2Phase;


OperatorConfiguration_t w_Operator1Config;
assign w_Operator1Config = i_Frequency1;

OperatorConfiguration_t w_Operator2Config;
assign w_Operator2Config = i_Frequency2;


phase_generator phasegen (
    .i_Clock          (i_Clock),
    .i_Reset          (i_Reset),
    .i_Operator1Config(w_Operator1Config),
    .i_Operator2Config(w_Operator2Config),
    .o_Operator1Phase (w_Operator1Phase),
    .o_Operator2Phase (w_Operator2Phase)
);


// verilator lint_off UNUSED
logic [23:0] w_Operator1Output;
logic [23:0] w_Operator2Output;
// verilator lint_on UNUSED

operator op1 (
    .i_Clock (i_Clock),
    // .i_Reset (i_Reset),
    .i_AmplitudeFactor(i_Amp1),
    .i_Phase (w_Operator1Phase),
    .o_Output(w_Operator1Output)
);

operator op2 (
    .i_Clock (i_Clock),
    // .i_Reset (i_Reset),
    .i_AmplitudeFactor(i_Amp2),
    .i_Phase (w_Operator2Phase),
    .o_Output(w_Operator2Output)
);

// TODO: Alternate mode to modulate instead of add
// verilator lint_off UNUSED
logic [24:0] w_SummedOutput;
// // TODO: How to just ignore the last bit? idk
// // verilator lint_on UNUSED
// assign w_SummedOutput =
//     {w_Operator1Output[23], w_Operator1Output[23:1]} +
//     {w_Operator2Output[23], w_Operator2Output[23:1]};
// assign o_Sample = w_SummedOutput[24:1];
// assign o_Sample = w_Operator1Output[23:0] + w_Operator2Output[23:0];
// assign o_Sample = w_Operator2Output[23:0];

assign o_Sample = w_Operator1Output[23:0] + w_Operator2Output[23:0];


// always_comb begin

//     w_SummedOutput =
//     {w_Operator1Output[23], w_Operator1Output[23:1]}

// end




always_ff @ (posedge i_Clock) begin
    if (i_Reset) begin
    end
    else begin
    end
end


endmodule
