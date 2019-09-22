
`include "voice.svh"

module voice (
    input logic i_Clock,
    input logic i_Reset,

    input VoiceRegisters_t r_Registers,

    output logic [23:0] o_Sample
);


// TODO: ADSR envelope for each operator amplitude


// TODO: Create in loop?
operator op1 (
    .i_Clock          (i_Clock),
    .i_Reset          (i_Reset),
    .i_Registers      (r_Registers.Operator[1]),
    .i_ModulatingPhase(0),  // TODO: Feedback
    .o_Output         (w_Operator1Output)
);

operator op2 (
    .i_Clock (i_Clock),
    .i_Reset          (i_Reset),
    .i_Registers      (r_Registers.Operator[2]),
    .i_ModulatingPhase(w_Op2ModulatingPhase),
    .o_Output         (w_Operator2Output)
);

logic signed [23:0] w_Operator1Output;
logic signed [23:0] w_Operator2Output;

logic signed [23:0] w_Op2ModulatingPhase;

always_comb begin

    case (r_Registers.Algorithm)
        0: w_Op2ModulatingPhase = w_Operator1Output;
        1: w_Op2ModulatingPhase = 0;
    endcase

    // TODO: ADSR on attack/release
    if (r_Registers.KeyOn) begin
        case (r_Registers.Algorithm)
            0: o_Sample = w_Operator2Output;
            1: o_Sample = w_Operator2Output + w_Operator1Output;
        endcase
    end
    else begin
        o_Sample = 0;
    end
end


endmodule
