
// TODO: Wrap DSP48E1 element using Vivado `ifdef
// https://www.xilinx.com/support/documentation/user_guides/ug479_7Series_DSP48E1.pdf
//


// DSP48E1 computes (A +/- B) * D + C
//
//
// We can explot this to create an operator feedback adder with
// a feedback coefficient B:
//
//    Po = Pi + B*Pf
//
// where Po is the phase sent into the operator, Pi is an input phase,
// and Pf is the feedback phase sent back from the operator's output



module multiplier (
    input i_Clock,

    // TODO
    // input i_Reset,

    input logic signed [17:0] i_Operand1,
    input logic signed [17:0] i_Operand2,
    output logic signed [17:0] o_Result
);

// I'll assume that the multiplier is pipelined in three stages for now.
logic [17:0] r_Result [1:0];

always_ff @ (posedge i_Clock) begin
    r_Result[0] <= i_Operand1 * i_Operand2;  // let it overflow
    r_Result[1] <= r_Result[0];
    o_Result <= r_Result[1];
end


endmodule
