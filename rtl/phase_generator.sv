
`include "registers.svh"

module phase_generator (
    input logic i_Clock,
    input logic i_Reset,
    input OperatorConfiguration_t i_Operator1Config,
    input OperatorConfiguration_t i_Operator2Config,

    output [23:0] o_Operator1Phase,
    output [23:0] o_Operator2Phase
);



// Still not really sure why summing the frequency up works to
// accumulate phase, but this seems to work under the following conditions.
// I suspect there is additional logic required when these don't line up
// as well.
//
// - Phase accumulator: 24 bits
// - Frequency values: 24 bits
// - Frequency values are multiples of 2^-8 Hz
// - Sample frequency is 2^16 Hz
//


always_ff @ (posedge i_Clock) begin
    if (i_Reset) begin
        o_Operator1Phase <= 0;
        o_Operator2Phase <= 0;
    end
    else begin
        o_Operator1Phase <= o_Operator1Phase + i_Operator1Config.Frequency;
        o_Operator2Phase <= o_Operator2Phase + i_Operator2Config.Frequency;
    end
end


endmodule
