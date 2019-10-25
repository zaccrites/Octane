
// Accumulate modulated phase according to algorithm

`include "synth.svh"

module stage_modulation (
    input i_Clock,

    output logic signed [15:0] o_ModulationPhase,

    input VoiceOperatorID_t i_VoiceOperator,
    output VoiceOperatorID_t o_VoiceOperator,

    output AlgorithmWord_t o_AlgorithmWord,

    input VoiceOperatorID_t i_OperatorWritebackID,
    input logic signed [15:0] i_OperatorWritebackValue,

    // configuration
    input logic i_AlgorithmWriteEnable,
    input VoiceOperatorID_t i_AlgorithmWriteAddr,
    input logic [7:0] i_AlgorithmWriteData

);


// Replicated to create more read ports.
`define OPERATOR_OUTPUT_MEMORY_READ_PORTS  7
logic signed [15:0] r_OperatorOutputMemory [`OPERATOR_OUTPUT_MEMORY_READ_PORTS] [`NUM_VOICE_OPERATORS];

logic signed [15:0] r_ModulationPhase [6:0];
logic signed [15:0] r_OperatorOutput [6:0];
AlgorithmWord_t r_AlgorithmWord [6:0];
VoiceOperatorID_t r_VoiceOperator [6:0];

AlgorithmWord_t r_Algorithm [`NUM_VOICE_OPERATORS];


// function logic signed [18:0] signExtendOperatorOutput(logic signed [15:0] operatorOutput);
//     return {{3{operatorOutput[15]}}, operatorOutput[15:0]};
// endfunction


integer i;
always_ff @ (posedge i_Clock) begin

    // TODO: Feedback

    if (i_AlgorithmWriteEnable)
        r_Algorithm[i_AlgorithmWriteAddr] <= i_AlgorithmWriteData;

    for (i = 0; i < `OPERATOR_OUTPUT_MEMORY_READ_PORTS; i = i + 1) begin
        r_OperatorOutputMemory[i][i_OperatorWritebackID] <= i_OperatorWritebackValue;
    end

    // Clock 1
    // ----------------------------------------------------------
    r_OperatorOutput[0] <= r_OperatorOutputMemory[0][i_VoiceOperator];

    r_AlgorithmWord[0] <= r_Algorithm[i_VoiceOperator];
    r_VoiceOperator[0] <= i_VoiceOperator;
    // ----------------------------------------------------------

    // Clock 2
    // ----------------------------------------------------------
    r_OperatorOutput[1] <= r_OperatorOutputMemory[1][r_VoiceOperator[0]];
    // r_ModulationPhase[1] <= r_AlgorithmWord[0].ModulateWithOP[0] ? signExtendOperatorOutput(r_OperatorOutput[0]) : 0;
    r_ModulationPhase[1] <= r_AlgorithmWord[0].ModulateWithOP[0] ? r_OperatorOutput[0] : 0;

    r_AlgorithmWord[1] <= r_AlgorithmWord[0];
    r_VoiceOperator[1] <= r_VoiceOperator[0];
    // ----------------------------------------------------------

    // Clocks 3-7
    // ----------------------------------------------------------
    for (i = 2; i <= 6; i = i + 1) begin
        r_OperatorOutput[i] <= r_OperatorOutputMemory[i][r_VoiceOperator[i - 1]];
        // r_ModulationPhase[i] <= r_ModulationPhase[i - 1] + (r_AlgorithmWord[i - 1].ModulateWithOP[i - 1] ? signExtendOperatorOutput(r_OperatorOutput[i - 1]) : 0);
        r_ModulationPhase[i] <= r_ModulationPhase[i - 1] + (r_AlgorithmWord[i - 1].ModulateWithOP[i - 1] ? r_OperatorOutput[i - 1] : 0);

        r_VoiceOperator[i] <= r_VoiceOperator[i - 1];
        r_AlgorithmWord[i] <= r_AlgorithmWord[i - 1];
    end
    // ----------------------------------------------------------

    // Clock 8
    // ----------------------------------------------------------
    o_ModulationPhase <= r_ModulationPhase[6][15:0];

    o_AlgorithmWord <= r_AlgorithmWord[6];
    o_VoiceOperator <= r_VoiceOperator[6];
    // ----------------------------------------------------------


    // TODO: Consider doing the phase accumulation in parallel here,
    // ending with a stage that does the modulated phase, the accumulated
    // phase, and possibly even the feedback phase in a single module.
    //
    // It may not be worth it though, since the throughput will still
    // be a sample on every clock and the only benefit would be slightly
    // reduced latency. In this application, latency doesn't matter much.


end

endmodule
