
`include "synth.svh"


module synth (
    input logic i_Clock,
    input logic i_Reset,

    input logic i_RegisterWriteEnable,
    input logic [15:0] i_RegisterWriteValue,
    // verilator lint_off UNUSED
    input logic [15:0] i_RegisterWriteNumber,
    // verilator lint_on UNUSED

    output logic o_SampleReady,
    output logic [15:0] o_Sample
);


// Voice-operator registers use the following 16-bit address scheme:
//   11 PPPPPP VVVVV OOO
//     - P (6 bits) represents the parameter type.
//     - V (5 bits) represents the zero-based voice number.
//     - O (3 bits) represents the zero-based operator number.
//
// Global registers use the following 16-bit address scheme:
//   10 PPPPPPPPPPPPPP
//     - P (14 bits) represents the parameter.
//


// TODO: Use full 16 bits for register numbers
logic [5:0] w_VoiceOperatorRegisterWriteParameter;
logic [5:0] w_VoiceOperatorRegisterWriteAddress;
assign w_VoiceOperatorRegisterWriteParameter = i_RegisterWriteNumber[13:8];
assign w_VoiceOperatorRegisterWriteAddress = i_RegisterWriteNumber[5:0];


function voiceOpRegWriteEnable;
    input logic [5:0] parameterBits;
begin
    voiceOpRegWriteEnable =
        i_RegisterWriteEnable && i_RegisterWriteNumber[14] == 1'b0 &&
        w_VoiceOperatorRegisterWriteParameter == parameterBits;

    if (voiceOpRegWriteEnable) begin
        $display("Writing register %x with value %x", i_RegisterWriteNumber, i_RegisterWriteValue);
    end
end
endfunction

logic w_SineTableWriteEnable;
logic w_NoteOnConfigWriteEnable;
logic [4:0] w_EnvelopeConfigWriteEnable;
logic w_AlgorithmWriteEnable;
logic w_PhaseStepWriteEnable;
logic w_FeedbackLevelConfigWriteEnable;

always_comb begin

    w_SineTableWriteEnable = i_RegisterWriteEnable && i_RegisterWriteNumber[14] == 1'b1;

    w_NoteOnConfigWriteEnable = voiceOpRegWriteEnable(6'h10);

    w_PhaseStepWriteEnable = voiceOpRegWriteEnable(6'h00);
    w_AlgorithmWriteEnable = voiceOpRegWriteEnable(6'h01);
    w_EnvelopeConfigWriteEnable[0] = voiceOpRegWriteEnable(6'h02);  // ATTACK LEVEL
    w_EnvelopeConfigWriteEnable[1] = voiceOpRegWriteEnable(6'h03);  // SUSTAIN LEVEL
    w_EnvelopeConfigWriteEnable[2] = voiceOpRegWriteEnable(6'h04);  // ATTACK RATE
    w_EnvelopeConfigWriteEnable[3] = voiceOpRegWriteEnable(6'h05);  // DECAY RATE
    w_EnvelopeConfigWriteEnable[4] = voiceOpRegWriteEnable(6'h06);  // RELEASE RATE
    w_FeedbackLevelConfigWriteEnable = voiceOpRegWriteEnable(6'h07);

end


/// Track the currently active voice operator
always_ff @ (posedge i_Clock) begin
    // Each clock cycle will compute a new value for a voice operator.
    // The order is the following:
    //
    // |    OP1     |    OP2     | ... |    OP7     |    OP8     |
    // | V1 ... V12 | V1 ... V12 | ... | V1 ... V12 | V1 ... V12 |
    // |            |            |     |            |            |
    // 0            12           24    72           84           96
    //
    if (i_Reset)
        r_VoiceOperator[0] <= 0;
    else
        r_VoiceOperator[0] <= r_VoiceOperator[0] + 1;
end


logic `VOICE_OPERATOR_ID r_VoiceOperator [5];
logic `ALGORITHM_WORD r_AlgorithmWord [5];
logic r_NoteOn [5];

logic signed [16:0] w_ModulatedPhase;
logic [15:0] w_RawPhase;


stage_phase_accumulator phase_accumulator (
    .i_Clock                     (i_Clock),
    .i_VoiceOperator             (r_VoiceOperator[0]),
    .o_VoiceOperator             (r_VoiceOperator[1]),
    .o_NoteOn                    (r_NoteOn[0]),

    .o_Phase                     (w_RawPhase),

    .i_NoteOnConfigWriteEnable   (w_NoteOnConfigWriteEnable),
    .i_PhaseStepConfigWriteEnable(w_PhaseStepWriteEnable),
    .i_ConfigWriteAddr  (w_VoiceOperatorRegisterWriteAddress[5:0]),
    .i_ConfigWriteData  (i_RegisterWriteValue)
);


stage_modulator modulator (
    .i_Clock          (i_Clock),

    .i_VoiceOperator  (r_VoiceOperator[1]),
    .o_VoiceOperator  (r_VoiceOperator[2]),

    .i_Phase       (w_RawPhase),
    .o_Phase       (w_ModulatedPhase),

    .i_NoteOn      (r_NoteOn[0]),
    .o_NoteOn      (r_NoteOn[1]),

    .o_AlgorithmWord      (r_AlgorithmWord[2]),

    .i_AlgorithmWriteEnable(w_AlgorithmWriteEnable),
    .i_FeedbackLevelConfigWriteEnable(w_FeedbackLevelConfigWriteEnable),
    .i_ConfigWriteAddr  (w_VoiceOperatorRegisterWriteAddress[5:0]),
    .i_ConfigWriteData  (i_RegisterWriteValue),

    .i_OperatorWritebackID   (w_OperatorWritebackID),
    .i_OperatorWritebackValue(w_OperatorWritebackValue)

);

always_ff @ (posedge i_Clock) begin
    // if (w_ModulatedPhase != 0) $display(w_ModulatedPhase);
    $display("note on? %d : voiceop (%d=%b) : raw_phase=%d", r_NoteOn[0], r_VoiceOperator[1], r_VoiceOperator[1], w_RawPhase);
end



logic signed [15:0] w_RawWaveform;

stage_waveform_generator waveform_generator (
    .i_Clock  (i_Clock),
    .i_Phase   (w_ModulatedPhase),
    .o_Waveform(w_RawWaveform),

    .i_NoteOn      (r_NoteOn[1]),
    .o_NoteOn      (r_NoteOn[2]),

    .i_SineTableWriteEnable (w_SineTableWriteEnable),
    .i_SineTableWriteAddress(i_RegisterWriteNumber[13:0]),
    .i_SineTableWriteValue  (i_RegisterWriteValue),

    .i_VoiceOperator(r_VoiceOperator[2]),
    .o_VoiceOperator(r_VoiceOperator[3]),

    .i_AlgorithmWord(r_AlgorithmWord[2]),
    .o_AlgorithmWord(r_AlgorithmWord[3])
);


// always_ff @ (posedge i_Clock) begin
//     $display(w_RawWaveform);
// end


logic signed [15:0] w_AttenuatedWaveform;

stage_envelope_attenuator envelope_attenuator (
    .i_Clock        (i_Clock),

    .i_NoteOn         (r_NoteOn[2]),

    .i_VoiceOperator  (r_VoiceOperator[3]),
    .o_VoiceOperator  (r_VoiceOperator[4]),

    .i_AlgorithmWord  (r_AlgorithmWord[3]),
    .o_AlgorithmWord  (r_AlgorithmWord[4]),

    .i_Waveform     (w_RawWaveform),
    .o_Waveform     (w_AttenuatedWaveform),

    .i_EnvelopeConfigWriteEnable(w_EnvelopeConfigWriteEnable),
    .i_ConfigWriteAddr          (w_VoiceOperatorRegisterWriteAddress[5:0]),
    .i_ConfigWriteData          (i_RegisterWriteValue)
);


logic `VOICE_OPERATOR_ID w_OperatorWritebackID;
logic signed [15:0] w_OperatorWritebackValue;
assign w_OperatorWritebackID = r_VoiceOperator[4];
assign w_OperatorWritebackValue = w_AttenuatedWaveform;


stage_sample_generator sample_generator (
    .i_Clock        (i_Clock),

    .i_VoiceOperator(r_VoiceOperator[4]),
    .i_AlgorithmWord (r_AlgorithmWord[4]),
    .i_OperatorOutput(w_AttenuatedWaveform),

    .o_SampleReady  (o_SampleReady),
    .o_Sample        (o_Sample)
);


endmodule
