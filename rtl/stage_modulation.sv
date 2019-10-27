
// Modulate phase according to an algorithm instruction word

`include "synth.svh"

module stage_modulation (
    input i_Clock,

    // Accepts raw unsigned phase and modulates to signed phase
    input logic unsigned [15:0] i_Phase,
    output logic signed [16:0] o_Phase,

    input VoiceOperatorID_t i_VoiceOperator,
    output VoiceOperatorID_t o_VoiceOperator,

    output AlgorithmWord_t o_AlgorithmWord,

    input VoiceOperatorID_t i_OperatorWritebackID,
    input logic signed [15:0] i_OperatorWritebackValue,

    // configuration
    input logic [1:0] i_AlgorithmWriteEnable,
    input VoiceOperatorID_t i_AlgorithmWriteAddr,
    // verilator lint_off UNUSED
    input logic [7:0] i_AlgorithmWriteData
    // verilator lint_on UNUSED

);


// Replicated to create more read ports.
`define OPERATOR_OUTPUT_MEMORY_READ_PORTS  7
logic signed [15:0] r_OperatorOutputMemory [`OPERATOR_OUTPUT_MEMORY_READ_PORTS] [`NUM_VOICE_OPERATORS];

// TODO: Fix modulation
// verilator lint_off UNUSED
logic signed [15:0] r_OperatorOutput [6:0];
// verilator lint_on UNUSED

logic signed [16:0] r_ModulatedPhase [6:0];
AlgorithmWord_t r_AlgorithmWord [6:0];
VoiceOperatorID_t r_VoiceOperator [6:0];

AlgorithmWord_t r_Algorithm [`NUM_VOICE_OPERATORS];


integer i;
always_ff @ (posedge i_Clock) begin

    // TODO: Feedback

    // TODO: Make sure Lattice tools figure out to use write enable mask
    if (i_AlgorithmWriteEnable[0]) begin
        r_Algorithm[i_AlgorithmWriteAddr].ModulateWithOP <= i_AlgorithmWriteData[6:0];
    end
    if (i_AlgorithmWriteEnable[1]) begin
        r_Algorithm[i_AlgorithmWriteAddr].IsACarrier <= i_AlgorithmWriteData[3];
        r_Algorithm[i_AlgorithmWriteAddr].NumCarriers <= i_AlgorithmWriteData[2:0];
    end

    for (i = 0; i < `OPERATOR_OUTPUT_MEMORY_READ_PORTS; i = i + 1) begin
        r_OperatorOutputMemory[i][i_OperatorWritebackID] <= i_OperatorWritebackValue;
    end

    // Clock 1
    // ----------------------------------------------------------
    r_OperatorOutput[0] <= r_OperatorOutputMemory[0][i_VoiceOperator];
    r_ModulatedPhase[0] <= $signed({1'b0, i_Phase});

    r_AlgorithmWord[0] <= r_Algorithm[i_VoiceOperator];
    r_VoiceOperator[0] <= i_VoiceOperator;
    // ----------------------------------------------------------

    // Clocks 2-7
    // ----------------------------------------------------------
    for (i = 1; i <= 6; i = i + 1) begin
        r_OperatorOutput[i] <= r_OperatorOutputMemory[i][r_VoiceOperator[i - 1]];
        // r_ModulatedPhase[i] <= r_ModulatedPhase[i - 1] + {r_OperatorOutput[i - 1][15], r_OperatorOutput[i - 1]};
        r_ModulatedPhase[i] <= r_ModulatedPhase[i - 1]; // + {r_OperatorOutput[i - 1][15], r_OperatorOutput[i - 1]};

        r_VoiceOperator[i] <= r_VoiceOperator[i - 1];
        r_AlgorithmWord[i] <= r_AlgorithmWord[i - 1];
    end
    // ----------------------------------------------------------

end


assign o_Phase = r_ModulatedPhase[6];
assign o_AlgorithmWord = r_AlgorithmWord[6];
assign o_VoiceOperator = r_VoiceOperator[6];


endmodule
