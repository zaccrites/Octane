
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
`define OPERATOR_OUTPUT_MEMORY_READ_PORTS  7
logic signed [15:0] r_OperatorOutputMemory [`OPERATOR_OUTPUT_MEMORY_READ_PORTS] [`NUM_VOICE_OPERATORS];

// verilator lint_off UNUSED


logic signed [15:0] r_OperatorOutput [7:0];
// logic signed [16:0] r_ModulatedPhase [7:0];
logic unsigned [15:0] r_ModulatedPhase [7:0];
AlgorithmWord_t r_AlgorithmWord [7:0];
VoiceOperatorID_t r_VoiceOperator [7:0];

AlgorithmWord_t r_Algorithm [`NUM_VOICE_OPERATORS];


integer i;
always_ff @ (posedge i_Clock) begin

    // TODO: Feedback

    if (i_AlgorithmWriteEnable)
        r_Algorithm[i_ConfigWriteAddr] <= i_ConfigWriteData[10:0];

    for (i = 0; i < `OPERATOR_OUTPUT_MEMORY_READ_PORTS; i = i + 1) begin
        r_OperatorOutputMemory[i][i_OperatorWritebackID] <= i_OperatorWritebackValue;
    end

    // Clock 1
    // ----------------------------------------------------------
    r_OperatorOutput[0] <= r_OperatorOutputMemory[0][i_VoiceOperator];
    // r_ModulatedPhase[0] <= $signed({1'b0, i_Phase});
    // r_ModulatedPhase[0] <= i_Phase;
    r_ModulatedPhase[0] <= {1'b0, i_Phase[15:1]};  // Do I need to divide modulators by number of modulators (plus one for the original phase, which is also divided)?

    r_AlgorithmWord[0] <= r_Algorithm[i_VoiceOperator];
    r_VoiceOperator[0] <= i_VoiceOperator;
    // ----------------------------------------------------------

    // Clocks 2-8
    // ----------------------------------------------------------
    for (i = 1; i <= 7; i = i + 1) begin
        if (i <= 6)
            r_OperatorOutput[i] <= r_OperatorOutputMemory[i][r_VoiceOperator[i - 1]];

        if (r_AlgorithmWord[i - 1].ModulateWithOP[i - 1]) begin
            // divide by a lot to avoid overflow (TODO: fix)
            // if (getVoiceID(r_VoiceOperator[i - 1]) == 0) $display("Adding modulation phase: %d", $signed({{1{r_OperatorOutput[i - 1][15]}}, r_OperatorOutput[i - 1][15:1]}));
            // r_ModulatedPhase[i] <= r_ModulatedPhase[i - 1] + {{1{r_OperatorOutput[i - 1][15]}}, r_OperatorOutput[i - 1][15:1]};
            // r_ModulatedPhase[i] <= r_ModulatedPhase[i - 1] + $unsigned(r_OperatorOutput[i - 1]);
            // r_ModulatedPhase[i] <= r_ModulatedPhase[i - 1] + {1'b0, $unsigned(r_OperatorOutput[i - 1][15:1])};
            r_ModulatedPhase[i] <= r_ModulatedPhase[i - 1];
        end
        else begin
            r_ModulatedPhase[i] <= r_ModulatedPhase[i - 1];
        end

        r_VoiceOperator[i] <= r_VoiceOperator[i - 1];
        r_AlgorithmWord[i] <= r_AlgorithmWord[i - 1];
    end
    // ----------------------------------------------------------

end

// TODO: Use unsigned phase only?
assign o_Phase = $signed({1'b0, r_ModulatedPhase[7]});
assign o_AlgorithmWord = r_AlgorithmWord[7];
assign o_VoiceOperator = r_VoiceOperator[7];


// logic signed [15:0] w_ModulationCompensationFactor;



endmodule
