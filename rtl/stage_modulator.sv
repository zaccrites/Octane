
// Modulate phase according to an algorithm instruction word

`include "synth.svh"

module stage_modulator (
    input i_Clock,

    // Accepts raw unsigned phase and modulates to signed phase
    // TODO: REMOVE THIS!
    // verilator lint_off UNUSED
    input logic unsigned [15:0] i_Phase,
    // verilator lint_on UNUSED
    output logic signed [16:0] o_Phase,

    input logic i_NoteOn,
    output logic o_NoteOn,

    input VoiceOperatorID_t i_VoiceOperator,
    output VoiceOperatorID_t o_VoiceOperator,

    output AlgorithmWord_t o_AlgorithmWord,

    input VoiceOperatorID_t i_OperatorWritebackID,
    input logic signed [15:0] i_OperatorWritebackValue,

    // configuration
    input logic i_AlgorithmWriteEnable,
    input VoiceOperatorID_t i_ConfigWriteAddr,
    // verilator lint_off UNUSED
    input logic [15:0] i_ConfigWriteData
    // verilator lint_on UNUSED

);


// Replicated to create more read ports.
logic signed [15:0] r_OperatorOutputMemory [7] [`NUM_VOICE_OPERATORS];

// verilator lint_off UNUSED


logic signed [15:0] r_OperatorOutput [7:0];
logic signed [16:0] r_ModulatedPhase [7:0];
AlgorithmWord_t r_AlgorithmWord [7:0];
VoiceOperatorID_t r_VoiceOperator [7:0];
logic r_NoteOn [7:0];

AlgorithmWord_t r_Algorithm [`NUM_VOICE_OPERATORS];


integer i;
always_ff @ (posedge i_Clock) begin

    if (i_AlgorithmWriteEnable) begin
        r_Algorithm[i_ConfigWriteAddr] <= i_ConfigWriteData[10:0];
    end

    if (i_OperatorWritebackValue != 0) begin
        // $display("Writing %d to operator slot [%0d.%0d]", i_OperatorWritebackValue, getVoiceID(i_OperatorWritebackID), getOperatorID(i_OperatorWritebackID));
    end

    for (i = 0; i < 7; i++) begin
        r_OperatorOutputMemory[i][i_OperatorWritebackID] <= i_OperatorWritebackValue;
    end

    // Clock 1
    // ----------------------------------------------------------
    r_OperatorOutput[0] <= r_OperatorOutputMemory[0][makeVoiceOperatorID(getVoiceID(i_VoiceOperator), 0)];
    r_ModulatedPhase[0] <= $signed({1'b0, i_Phase});
    // r_ModulatedPhase[0] <= {1'b0, i_Phase};
    // r_ModulatedPhase[0] <= {1'b0, i_Phase[15:1]};  // Do I need to divide modulators by number of modulators (plus one for the original phase, which is also divided)?

    r_NoteOn[0] <= i_NoteOn;
    r_AlgorithmWord[0] <= r_Algorithm[i_VoiceOperator];
    r_VoiceOperator[0] <= i_VoiceOperator;
    // ----------------------------------------------------------

    // Clocks 2-8
    // ----------------------------------------------------------
    for (i = 1; i <= 6; i++) begin
        // r_OperatorOutput[i] <= r_OperatorOutputMemory[i][makeVoiceOperatorID(getVoiceID(r_VoiceOperator[i - 1]), i)];
    end
    r_OperatorOutput[1] <= r_OperatorOutputMemory[1][makeVoiceOperatorID(getVoiceID(r_VoiceOperator[0]), 1)];
    r_OperatorOutput[2] <= r_OperatorOutputMemory[2][makeVoiceOperatorID(getVoiceID(r_VoiceOperator[1]), 2)];
    r_OperatorOutput[3] <= r_OperatorOutputMemory[3][makeVoiceOperatorID(getVoiceID(r_VoiceOperator[2]), 3)];
    r_OperatorOutput[4] <= r_OperatorOutputMemory[4][makeVoiceOperatorID(getVoiceID(r_VoiceOperator[3]), 4)];
    r_OperatorOutput[5] <= r_OperatorOutputMemory[5][makeVoiceOperatorID(getVoiceID(r_VoiceOperator[4]), 5)];
    r_OperatorOutput[6] <= r_OperatorOutputMemory[6][makeVoiceOperatorID(getVoiceID(r_VoiceOperator[5]), 6)];

    for (i = 1; i <= 7; i++) begin
        // if (r_AlgorithmWord[i - 1].ModulateWithOP[i - 1]) begin
        if (r_AlgorithmWord[i - 1].ModulateWithOP[i - 1]) begin
            // if (getVoiceID(r_VoiceOperator[i - 1]) == 0) $display("Adding modulation phase: %d", $signed({{1{r_OperatorOutput[i - 1][15]}}, r_OperatorOutput[i - 1][15:1]}));
            // r_ModulatedPhase[i] <= r_ModulatedPhase[i - 1] + {{1{r_OperatorOutput[i - 1][15]}}, r_OperatorOutput[i - 1][15:1]};
            // r_ModulatedPhase[i] <= r_ModulatedPhase[i - 1] + $unsigned(r_OperatorOutput[i - 1]);
            // r_ModulatedPhase[i] <= r_ModulatedPhase[i - 1] + {1'b0, $unsigned(r_OperatorOutput[i - 1][15:1])};

            // if (r_OperatorOutput[i - 1] != 0)
            //     $display(
            //         "r_VoiceOperator[%0d] = %0d.%0d, r_OperatorOutput[%0d] = %d, r_AlgorithmWord[%0d] = %b",
            //         i - 1, getVoiceID(r_VoiceOperator[i - 1]), getOperatorID(r_VoiceOperator[i - 1]),
            //         i - 1, r_OperatorOutput[i - 1],
            //         i - 1, r_AlgorithmWord[i - 1]
            //     );

            r_ModulatedPhase[i] <= r_ModulatedPhase[i - 1] + r_OperatorOutput[i - 1];
            // r_ModulatedPhase[i] <= r_ModulatedPhase[i - 1];
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
