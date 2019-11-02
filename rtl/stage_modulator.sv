
// Modulate phase according to an algorithm instruction word

`include "synth.svh"

module stage_modulator (
    input i_Clock,

    // Accepts raw unsigned phase and modulates to signed phase
    // TODO: REMOVE THIS!
    // verilator lint_off UNUSED
    input logic [15:0] i_Phase,
    // verilator lint_on UNUSED
    output logic signed [16:0] o_Phase,

    input logic i_NoteOn,
    output logic o_NoteOn,

    input logic `VOICE_OPERATOR_ID i_VoiceOperator,
    output logic `VOICE_OPERATOR_ID o_VoiceOperator,

    output logic `ALGORITHM_WORD o_AlgorithmWord,

    input logic `VOICE_OPERATOR_ID i_OperatorWritebackID,
    input logic signed [15:0] i_OperatorWritebackValue,

    input logic i_AlgorithmWriteEnable,
    input logic `VOICE_OPERATOR_ID i_ConfigWriteAddr,
    // verilator lint_off UNUSED
    input logic [15:0] i_ConfigWriteData
    // verilator lint_on UNUSED

);

// Replicated to create more read ports.
logic signed [15:0] r_OperatorOutputMemory [7] [`NUM_VOICE_OPERATORS];

logic signed [15:0] r_OperatorOutput [7:0];
logic signed [16:0] r_ModulatedPhase [7:0];
logic `ALGORITHM_WORD r_AlgorithmWord [7:0];
logic `VOICE_OPERATOR_ID r_VoiceOperator [7:0];
logic r_NoteOn [7:0];

logic `ALGORITHM_WORD r_Algorithm [`NUM_VOICE_OPERATORS];


integer i;
always_ff @ (posedge i_Clock) begin

    if (i_AlgorithmWriteEnable) begin
        r_Algorithm[i_ConfigWriteAddr] <= i_ConfigWriteData[10:0];
    end

    for (i = 0; i < 7; i++) begin
        r_OperatorOutputMemory[i][i_OperatorWritebackID] <= i_OperatorWritebackValue;
    end

    // Each step of this represents a specific operator's output, so the
    // read port is hard coded to the specific operator within the current voice.
    r_OperatorOutput[0] <= r_OperatorOutputMemory[0][makeVoiceOperatorID(getVoiceID(i_VoiceOperator), 0)];
    r_OperatorOutput[1] <= r_OperatorOutputMemory[1][makeVoiceOperatorID(getVoiceID(r_VoiceOperator[0]), 1)];
    r_OperatorOutput[2] <= r_OperatorOutputMemory[2][makeVoiceOperatorID(getVoiceID(r_VoiceOperator[1]), 2)];
    r_OperatorOutput[3] <= r_OperatorOutputMemory[3][makeVoiceOperatorID(getVoiceID(r_VoiceOperator[2]), 3)];
    r_OperatorOutput[4] <= r_OperatorOutputMemory[4][makeVoiceOperatorID(getVoiceID(r_VoiceOperator[3]), 4)];
    r_OperatorOutput[5] <= r_OperatorOutputMemory[5][makeVoiceOperatorID(getVoiceID(r_VoiceOperator[4]), 5)];
    r_OperatorOutput[6] <= r_OperatorOutputMemory[6][makeVoiceOperatorID(getVoiceID(r_VoiceOperator[5]), 6)];

    // Clock 1
    // ----------------------------------------------------------
    r_ModulatedPhase[0] <= $signed({1'b0, i_Phase});

    r_NoteOn[0] <= i_NoteOn;
    r_AlgorithmWord[0] <= r_Algorithm[i_VoiceOperator];
    r_VoiceOperator[0] <= i_VoiceOperator;
    // ----------------------------------------------------------

    // Clocks 2-8
    // ----------------------------------------------------------
    for (i = 1; i <= 7; i++) begin
        if (getModulateWithOP(r_AlgorithmWord[i - 1], i - 1)) begin
            r_ModulatedPhase[i] <= r_ModulatedPhase[i - 1] + r_OperatorOutput[i - 1];
        end
        else begin
            r_ModulatedPhase[i] <= r_ModulatedPhase[i - 1];
        end

        r_NoteOn[i] <= r_NoteOn[i - 1];
        r_VoiceOperator[i] <= r_VoiceOperator[i - 1];
        r_AlgorithmWord[i] <= r_AlgorithmWord[i - 1];
    end
    // ----------------------------------------------------------

end

assign o_Phase = r_ModulatedPhase[7];
assign o_AlgorithmWord = r_AlgorithmWord[7];
assign o_VoiceOperator = r_VoiceOperator[7];
assign o_NoteOn = r_NoteOn[7];


endmodule
