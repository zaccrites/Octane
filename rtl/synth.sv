
`include "synth.svh"


module synth (
    input logic i_Clock,
    input logic i_Reset,

    input logic i_SPI_SCK,
    input logic i_SPI_MOSI,
    output logic o_SPI_MISO

    // TODO: Use SPI interface for input and output (collect bits into a register)
    // input logic i_RegisterWriteEnable,
    // input logic [15:0] w_RegisterWriteNumber,
    // input logic [15:0] w_RegisterWriteValue,

    // output logic signed [15:0] o_Sample,
    // output logic o_SampleReady

);



logic w_RegisterWriteEnable;
logic [14:0] w_RegisterWriteNumber;
logic [15:0] w_RegisterWriteValue;

spi spi0 (
    .i_Clock              (i_Clock),
    .i_SampleReady        (w_SampleReady),
    .i_SampleToOutput     (w_Sample),
    .i_SPI_SCK            (i_SPI_SCK),
    .i_SPI_MOSI           (i_SPI_MOSI),
    .o_SPI_MISO           (o_SPI_MISO),
    .o_RegisterWriteEnable (w_RegisterWriteEnable),
    .o_RegisterWriteNumber(w_RegisterWriteNumber),
    .o_RegisterWriteValue (w_RegisterWriteValue)
);


// Voice-operator registers use the following 16-bit address scheme:
//   11 PPPPPP VVVVV OOO
//     - P (6 bits) represents the parameter type.
//     - V (5 bits) represents the zero-based voice number.
//     - O (3 bits) represents the zero-based operator number.
//
// Global registers use the following 16-bit address scheme:
//   10 PPPPPP RRRRRRRR
//     - P (6 bits) represents the parameter type.
//     - R (8 bits) are reserved.
//
logic [6:0] w_RegisterWriteParameter;
assign w_RegisterWriteParameter = w_RegisterWriteNumber[14:8];
logic [7:0] w_VoiceOpRegWriteIndex;
assign w_VoiceOpRegWriteIndex = w_RegisterWriteNumber[7:0];

function logic voiceOpRegWriteEnable(logic [5:0] parameterBits);
    return w_RegisterWriteEnable && w_RegisterWriteParameter == {1'b1, parameterBits};
endfunction

function logic globalRegWriteEnable(logic [5:0] parameterBits);
    return w_RegisterWriteEnable && w_RegisterWriteParameter == {1'b0, parameterBits};
endfunction

logic [1:0] w_NoteOnConfigWriteEnable;
logic [4:0] w_EnvelopeConfigWriteEnable;
logic w_AlgorithmWriteEnable;
logic w_PhaseStepWriteEnable;

always_comb begin

    w_NoteOnConfigWriteEnable[0] = globalRegWriteEnable(6'h00);
    w_NoteOnConfigWriteEnable[1] = globalRegWriteEnable(6'h01);

    w_PhaseStepWriteEnable = voiceOpRegWriteEnable(6'h00);
    w_AlgorithmWriteEnable = voiceOpRegWriteEnable(6'h01);
    w_EnvelopeConfigWriteEnable[0] = voiceOpRegWriteEnable(6'h01);  // ATTACK LEVEL
    w_EnvelopeConfigWriteEnable[1] = voiceOpRegWriteEnable(6'h02);  // SUSTAIN LEVEL
    w_EnvelopeConfigWriteEnable[2] = voiceOpRegWriteEnable(6'h03);  // ATTACK RATE
    w_EnvelopeConfigWriteEnable[3] = voiceOpRegWriteEnable(6'h04);  // DECAY RATE
    w_EnvelopeConfigWriteEnable[4] = voiceOpRegWriteEnable(6'h05);  // RELEASE RATE

end

always begin
    if (w_PhaseStepWriteEnable) $display("Writing %x to phase_step (%x)", w_RegisterWriteValue, w_RegisterWriteNumber);
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
    .i_ConfigWriteData  (w_RegisterWriteValue)
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
    .i_ConfigWriteData  (w_RegisterWriteValue),

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
    .i_ConfigWriteData          (w_RegisterWriteValue)
);



// TODO
VoiceOperatorID_t w_OperatorWritebackID;
logic signed [15:0] w_OperatorWritebackValue;
assign w_OperatorWritebackID = r_VoiceOperator[4];
assign w_OperatorWritebackValue = w_AttenuatedWaveform;


logic w_SampleReady;
logic signed [15:0] w_Sample;

stage_sample_generator sample_generator (
    .i_Clock        (i_Clock),

    .i_VoiceOperator(r_VoiceOperator[4]),
    .i_AlgorithmWord (r_AlgorithmWord[4]),
    .i_OperatorOutput(w_AttenuatedWaveform),

    .o_SampleReady  (w_SampleReady),
    .o_Sample        (w_Sample)
);


endmodule
