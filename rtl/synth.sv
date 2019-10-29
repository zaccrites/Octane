
`include "synth.svh"


module synth (
    input logic i_Clock,
    input logic i_Reset,

    // TODO: Use SPI interface for input and output (collect bits into a register)
    input logic i_RegisterWriteEnable,
    input logic [15:0] i_RegisterWriteNumber,
    input logic [15:0] i_RegisterWriteValue,

    output logic signed [15:0] o_Sample,
    output logic o_SampleReady

);


// Register assignment
//
//
// Register numbers:
//
//
// Voice-operator registers use the following 16-bit address scheme:
//
// SS PPPPPP OOO VVVVV
//
// With each field representing the following:
//
// S (2 bits) represents the scope of the configuration register:
//
//   11 - Voice operator
//   10 - Voice
//   01 - Global
//   00 - <reserved>
//
// P (6 bits) represents the parameter type.
//
// O (3 bits) represents the zero-based operator number.
//
// V (5 bits) represents the zero-based voice number.
//
//
logic [7:0] w_RegisterWriteCategory;  // TODO: Find better name
assign w_RegisterWriteCategory = i_RegisterWriteNumber[15:8];
logic [7:0] w_VoiceOpRegWriteIndex;
assign w_VoiceOpRegWriteIndex = i_RegisterWriteNumber[7:0];

function logic regWriteEnable(logic [7:0] category);
    return i_RegisterWriteEnable && w_RegisterWriteCategory == category;
endfunction

logic [1:0] w_NoteOnConfigWriteEnable;
logic [4:0] w_EnvelopeConfigWriteEnable;
logic w_AlgorithmWriteEnable;
logic w_PhaseStepWriteEnable;

always_comb begin

    // GLOBAL CONFIGURATION
    w_NoteOnConfigWriteEnable[0] = regWriteEnable({2'b01, 6'h00});
    w_NoteOnConfigWriteEnable[1] = regWriteEnable({2'b01, 6'h01});

    // VOICE CONFIGURATION
    // (none yet)

    // VOICE OPERATOR CONFIGURATION
    w_PhaseStepWriteEnable = regWriteEnable({2'b11, 6'h00});
    w_AlgorithmWriteEnable = regWriteEnable({2'b11, 6'h01});

    w_EnvelopeConfigWriteEnable[0] = regWriteEnable({2'b11, 6'h01});  // ATTACK LEVEL
    w_EnvelopeConfigWriteEnable[1] = regWriteEnable({2'b11, 6'h02});  // SUSTAIN LEVEL
    w_EnvelopeConfigWriteEnable[2] = regWriteEnable({2'b11, 6'h03});  // ATTACK RATE
    w_EnvelopeConfigWriteEnable[3] = regWriteEnable({2'b11, 6'h04});  // DECAY RATE
    w_EnvelopeConfigWriteEnable[4] = regWriteEnable({2'b11, 6'h05});  // RELEASE RATE

end


// TODO: Move into its own module?
/// Track the currently active voice operator
always_ff @ (posedge i_Clock) begin
    // Each clock cycle will compute a new value for a voice operator.
    // The order is the following:
    //
    // |    OP1     |    OP2     | ... |    OP7     |    OP8     |
    // | V1 ... V32 | V1 ... V32 | ... | V1 ... V32 | V1 ... V32 |
    // |            |            |     |            |            |
    // 0            32           64    192          224          256
    //
    if (i_Reset)
        r_VoiceOperator[0] <= 0;
    else
        r_VoiceOperator[0] <= r_VoiceOperator[0] + 1;
end


// TODO: Trim these down
VoiceOperatorID_t r_VoiceOperator [31:0];
AlgorithmWord_t r_AlgorithmWord [31:0];


logic signed [16:0] w_ModulatedPhase;
logic unsigned [15:0] w_RawPhase;


stage_phase_accumulator phase_accumulator (
    .i_Clock                     (i_Clock),
    .i_VoiceOperator             (r_VoiceOperator[0]),
    .o_VoiceOperator             (r_VoiceOperator[1]),

    .o_Phase                     (w_RawPhase),

    .i_PhaseStepConfigWriteEnable(w_PhaseStepWriteEnable),
    .i_ConfigWriteAddr  (w_VoiceOpRegWriteIndex),
    .i_ConfigWriteData  (i_RegisterWriteValue)
);


stage_modulator modulator (
    .i_Clock          (i_Clock),

    .i_VoiceOperator  (r_VoiceOperator[1]),
    .o_VoiceOperator  (r_VoiceOperator[2]),

    .i_Phase       (w_RawPhase),
    .o_Phase       (w_ModulatedPhase),


    .o_AlgorithmWord      (r_AlgorithmWord[2]),

    // TODO: Set these if register written above
    .i_AlgorithmWriteEnable(w_AlgorithmWriteEnable),
    .i_ConfigWriteAddr  (w_VoiceOpRegWriteIndex),
    .i_ConfigWriteData  (i_RegisterWriteValue),

    .i_OperatorWritebackID   (w_OperatorWritebackID),
    .i_OperatorWritebackValue(w_OperatorWritebackValue)

);


logic signed [15:0] w_RawWaveform;

stage_waveform_generator waveform_generator (
    .i_Clock  (i_Clock),
    .i_Phase   (w_ModulatedPhase),
    .o_Waveform(w_RawWaveform),

    .i_VoiceOperator(r_VoiceOperator[2]),
    .o_VoiceOperator(r_VoiceOperator[3]),

    .i_AlgorithmWord(r_AlgorithmWord[2]),
    .o_AlgorithmWord(r_AlgorithmWord[3])
);


logic signed [15:0] w_AttenuatedWaveform;

stage_envelope_attenuator envelope_attenuator (
    .i_Clock        (i_Clock),

    .i_VoiceOperator  (r_VoiceOperator[3]),
    .o_VoiceOperator  (r_VoiceOperator[4]),

    .i_AlgorithmWord  (r_AlgorithmWord[3]),
    .o_AlgorithmWord  (r_AlgorithmWord[4]),

    .i_Waveform     (w_RawWaveform),
    .o_Waveform     (w_AttenuatedWaveform),

    .i_EnvelopeConfigWriteEnable(w_EnvelopeConfigWriteEnable),
    .i_NoteOnConfigWriteEnable  (w_NoteOnConfigWriteEnable),
    .i_ConfigWriteAddr          (w_VoiceOpRegWriteIndex),
    .i_ConfigWriteData          (i_RegisterWriteValue)
);



// TODO
VoiceOperatorID_t w_OperatorWritebackID;
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
