
// Modulate phase according to an algorithm instruction word

`include "synth.svh"

module stage_modulator (
    input i_Clock,

    // Accepts raw unsigned phase and modulates to signed phase
    input logic [15:0] i_Phase,
    output logic signed [16:0] o_Phase,

    input logic i_NoteOn,
    output logic o_NoteOn,

    input logic `VOICE_OPERATOR_ID i_VoiceOperator,
    output logic `VOICE_OPERATOR_ID o_VoiceOperator,

    output logic `ALGORITHM_WORD o_AlgorithmWord,

    input logic `VOICE_OPERATOR_ID i_OperatorWritebackID,
    input logic signed [15:0] i_OperatorWritebackValue,

    input logic i_AlgorithmWriteEnable,
    input logic i_FeedbackLevelConfigWriteEnable,
    input logic `VOICE_OPERATOR_ID i_ConfigWriteAddr,
    // verilator lint_off UNUSED
    input logic [15:0] i_ConfigWriteData
    // verilator lint_on UNUSED

);

// Replicated to create more read ports.
logic signed [15:0] r_OperatorOutputMemory [7] [`NUM_VOICE_OPERATORS];

logic `ALGORITHM_WORD r_Algorithm [`NUM_VOICE_OPERATORS];


logic signed [15:0] r_OperatorOutput [10];
logic signed [16:0] r_ModulatedPhase [10];
logic `ALGORITHM_WORD r_AlgorithmWord [10];
logic `VOICE_OPERATOR_ID r_VoiceOperator [10];
logic r_NoteOn [10];


`define FEEDBACK_LEVEL  [7:0]
logic `FEEDBACK_LEVEL r_FeedbackLevelConfig [`NUM_VOICE_OPERATORS];

// See US Patent 4,249,447 to see how averaging the previous two feedback
// values can be used to implement an "anti-hunting" filter (esp. FIG 7(a) and 7(b))
// https://patentimages.storage.googleapis.com/9e/ba/a4/fb43500a975d67/US4249447.pdf
logic signed [15:0] r_OperatorFeedbackMemory [2] [`NUM_VOICE_OPERATORS];


logic `FEEDBACK_LEVEL r_FeedbackLevel[10];
logic signed [15:0] r_RawFeedbackPhase [10];
logic signed [24:0] r_FeedbackPhaseProduct [9:2];

logic signed [15:0] r_RawFeedbackPhase0;
logic signed [15:0] r_RawFeedbackPhase1;
// Bit 0 isn't used because the two are being averaged
// verilator lint_off UNUSED
logic signed [16:0] w_CombinedFeedbackPhase;
// verilator lint_on UNUSED
assign w_CombinedFeedbackPhase = r_RawFeedbackPhase0 + r_RawFeedbackPhase1;


integer i;
always_ff @ (posedge i_Clock) begin

    if (i_AlgorithmWriteEnable) r_Algorithm[i_ConfigWriteAddr] <= i_ConfigWriteData[10:0];
    if (i_FeedbackLevelConfigWriteEnable) r_FeedbackLevelConfig[i_ConfigWriteAddr] <= i_ConfigWriteData[7:0];

    for (i = 0; i < 7; i++) begin
        r_OperatorOutputMemory[i][i_OperatorWritebackID] <= i_OperatorWritebackValue;
    end
    r_OperatorFeedbackMemory[0][i_OperatorWritebackID] <= i_OperatorWritebackValue;

    // Each step of this represents a specific operator's output, so the
    // read port is hard coded to the specific operator within the current voice.
    r_OperatorOutput[0] <= r_OperatorOutputMemory[0][makeVoiceOperatorID(getVoiceID(r_VoiceOperator[0]), 0)];
    r_OperatorOutput[1] <= r_OperatorOutputMemory[1][makeVoiceOperatorID(getVoiceID(r_VoiceOperator[1]), 1)];
    r_OperatorOutput[2] <= r_OperatorOutputMemory[2][makeVoiceOperatorID(getVoiceID(r_VoiceOperator[2]), 2)];
    r_OperatorOutput[3] <= r_OperatorOutputMemory[3][makeVoiceOperatorID(getVoiceID(r_VoiceOperator[3]), 3)];
    r_OperatorOutput[4] <= r_OperatorOutputMemory[4][makeVoiceOperatorID(getVoiceID(r_VoiceOperator[4]), 4)];
    r_OperatorOutput[5] <= r_OperatorOutputMemory[5][makeVoiceOperatorID(getVoiceID(r_VoiceOperator[5]), 5)];
    r_OperatorOutput[6] <= r_OperatorOutputMemory[6][makeVoiceOperatorID(getVoiceID(r_VoiceOperator[6]), 6)];

    // Clock 1
    // ----------------------------------------------------------
    r_ModulatedPhase[0] <= $signed({1'b0, i_Phase});
    r_NoteOn[0] <= i_NoteOn;
    r_VoiceOperator[0] <= i_VoiceOperator;
    r_RawFeedbackPhase0 <= r_OperatorFeedbackMemory[0][i_VoiceOperator];
    r_RawFeedbackPhase1 <= r_OperatorFeedbackMemory[1][i_VoiceOperator];
    // ----------------------------------------------------------

    // Clock 2
    // ----------------------------------------------------------
    r_RawFeedbackPhase[1] <= w_CombinedFeedbackPhase[16:1];
    r_ModulatedPhase[1] <= r_ModulatedPhase[0];
    r_AlgorithmWord[1] <= r_Algorithm[r_VoiceOperator[0]];
    r_FeedbackLevel[1] <= r_FeedbackLevelConfig[r_VoiceOperator[0]];
    r_NoteOn[1] <= r_NoteOn[0];
    r_VoiceOperator[1] <= r_VoiceOperator[0];

    r_OperatorFeedbackMemory[1][r_VoiceOperator[0]] <= r_RawFeedbackPhase0;
    // ----------------------------------------------------------

    // Clocks 3-10
    // ----------------------------------------------------------
    // Pipeline an (8 bit unsigned)x(16 bit signed) multiply by repeated addition
    r_FeedbackPhaseProduct[2] <=                              r_FeedbackLevel[1][0] ? {{9{r_RawFeedbackPhase[1][15]}}, r_RawFeedbackPhase[1]}       : 0;
    r_FeedbackPhaseProduct[3] <= r_FeedbackPhaseProduct[2] + (r_FeedbackLevel[2][1] ? {{8{r_RawFeedbackPhase[2][15]}}, r_RawFeedbackPhase[2], 1'b0} : 0);
    r_FeedbackPhaseProduct[4] <= r_FeedbackPhaseProduct[3] + (r_FeedbackLevel[3][2] ? {{7{r_RawFeedbackPhase[3][15]}}, r_RawFeedbackPhase[3], 2'b0} : 0);
    r_FeedbackPhaseProduct[5] <= r_FeedbackPhaseProduct[4] + (r_FeedbackLevel[4][3] ? {{6{r_RawFeedbackPhase[4][15]}}, r_RawFeedbackPhase[4], 3'b0} : 0);
    r_FeedbackPhaseProduct[6] <= r_FeedbackPhaseProduct[5] + (r_FeedbackLevel[5][4] ? {{5{r_RawFeedbackPhase[5][15]}}, r_RawFeedbackPhase[5], 4'b0} : 0);
    r_FeedbackPhaseProduct[7] <= r_FeedbackPhaseProduct[6] + (r_FeedbackLevel[6][5] ? {{4{r_RawFeedbackPhase[6][15]}}, r_RawFeedbackPhase[6], 5'b0} : 0);
    r_FeedbackPhaseProduct[8] <= r_FeedbackPhaseProduct[7] + (r_FeedbackLevel[7][6] ? {{3{r_RawFeedbackPhase[7][15]}}, r_RawFeedbackPhase[7], 6'b0} : 0);
    r_FeedbackPhaseProduct[9] <= r_FeedbackPhaseProduct[8] + (r_FeedbackLevel[8][7] ? {{2{r_RawFeedbackPhase[8][15]}}, r_RawFeedbackPhase[8], 7'b0} : 0);

    for (i = 2; i <= 8; i++) begin
        r_FeedbackLevel[i] <= r_FeedbackLevel[i - 1];
        r_RawFeedbackPhase[i] <= r_RawFeedbackPhase[i - 1];
    end

    r_ModulatedPhase[2] <= r_ModulatedPhase[1] + (getModulateWithOP(r_AlgorithmWord[1], 0) ? {r_OperatorOutput[0][15], r_OperatorOutput[0]} : 0);
    r_ModulatedPhase[3] <= r_ModulatedPhase[2] + (getModulateWithOP(r_AlgorithmWord[2], 1) ? {r_OperatorOutput[1][15], r_OperatorOutput[1]} : 0);
    r_ModulatedPhase[4] <= r_ModulatedPhase[3] + (getModulateWithOP(r_AlgorithmWord[3], 2) ? {r_OperatorOutput[2][15], r_OperatorOutput[2]} : 0);
    r_ModulatedPhase[5] <= r_ModulatedPhase[4] + (getModulateWithOP(r_AlgorithmWord[4], 3) ? {r_OperatorOutput[3][15], r_OperatorOutput[3]} : 0);
    r_ModulatedPhase[6] <= r_ModulatedPhase[5] + (getModulateWithOP(r_AlgorithmWord[5], 4) ? {r_OperatorOutput[4][15], r_OperatorOutput[4]} : 0);
    r_ModulatedPhase[7] <= r_ModulatedPhase[6] + (getModulateWithOP(r_AlgorithmWord[6], 5) ? {r_OperatorOutput[5][15], r_OperatorOutput[5]} : 0);
    r_ModulatedPhase[8] <= r_ModulatedPhase[7] + (getModulateWithOP(r_AlgorithmWord[7], 6) ? {r_OperatorOutput[6][15], r_OperatorOutput[6]} : 0);
    r_ModulatedPhase[9] <= r_ModulatedPhase[8];  // OP8 can never be a modulator

    for (i = 2; i <= 9; i++) begin
        r_NoteOn[i] <= r_NoteOn[i - 1];
        r_VoiceOperator[i] <= r_VoiceOperator[i - 1];
        r_AlgorithmWord[i] <= r_AlgorithmWord[i - 1];
    end
    // ----------------------------------------------------------

    // Clock 11
    // ----------------------------------------------------------
    o_Phase <= r_ModulatedPhase[9] + {{2{r_FeedbackPhaseProduct[9][24]}}, r_FeedbackPhaseProduct[9][23:9]};
    o_NoteOn <= r_NoteOn[9];
    o_VoiceOperator <= r_VoiceOperator[9];
    o_AlgorithmWord <= r_AlgorithmWord[9];
    // ----------------------------------------------------------


end

endmodule
