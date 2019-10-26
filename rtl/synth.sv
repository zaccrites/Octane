
`include "synth.svh"


module synth (
    input logic i_Clock,
    input logic i_Reset,

    // TODO: Use SPI interface for input and output (collect bits into a register)
    input logic i_RegisterWriteEnable,
    input logic [15:0] i_RegisterWriteNumber,
    input logic [7:0] i_RegisterWriteValue,

    output logic signed [15:0] o_Sample,
    output logic o_SampleReady

);


// verilator lint_off UNUSED




/// Voice operator configuration registers

/// Frequency information
logic unsigned [15:0] r_PhaseStep [256];

/// Waveform type and parameters
logic [15:0] r_Waveform  [256];

/// Envelope level registers
logic unsigned [7:0] r_EnvelopeL1 [256];
logic unsigned [7:0] r_EnvelopeL2 [256];
logic unsigned [7:0] r_EnvelopeL3 [256];
logic unsigned [7:0] r_EnvelopeL4 [256];

/// Envelope rate registers
logic unsigned [7:0] r_EnvelopeR1 [256];
logic unsigned [7:0] r_EnvelopeR2 [256];
logic unsigned [7:0] r_EnvelopeR3 [256];
logic unsigned [7:0] r_EnvelopeR4 [256];

/// Algorithm instruction words
logic [7:0] r_Algorithm [256];



/// Voice configuration registers
logic r_NoteOn [32];

// TODO: This could be included in the algorithm ROM (though perhaps as a separate BRAM to avoid a structural hazard)
/// The factor used to compensate for multiple carrier operators
logic unsigned [15:0] r_CarrierComp [32];








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
//   01 - <reserved>
//   00 - <reserved>
//
// P (6 bits) represents the parameter type.
// The following are defined:
//
// Voice
//  00h: Note on
//  01h: Algorithm
//
// Voice operator
//  00h: Phase step (high)
//  01h: Phase step (low)
//  02h: Waveform (high)
//  03h: Waveform (low)
//
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
logic [4:0] w_VoiceRegWriteIndex;
assign w_VoiceRegWriteIndex = i_RegisterWriteNumber[4:0];
//


function logic regWriteEnable(logic [7:0] category);
    return i_RegisterWriteEnable && w_RegisterWriteCategory == category;
endfunction


logic [1:0] w_AlgorithmWriteEnable;
logic [1:0] w_PhaseStepWriteEnable;
always_comb begin

    w_PhaseStepWriteEnable[0] = regWriteEnable({2'b11, 6'h00});  // HIGH
    w_PhaseStepWriteEnable[1] = regWriteEnable({2'b11, 6'h01});  // LOW

    w_AlgorithmWriteEnable[0] = regWriteEnable({2'b11, 6'h0c});  // HIGH
    w_AlgorithmWriteEnable[1] = regWriteEnable({2'b11, 6'h0d});  // LOW

end


always_ff @ (posedge i_Clock) begin

    // if (i_RegisterWriteEnable) begin
    //     case (i_RegisterWriteNumber[15:8])
    //         // TODO: Rearrange these
    //         //
    //         //
    //         {2'b11, 6'h00}: r_PhaseStep[w_VoiceOpRegWriteIndex][15:8] <= i_RegisterWriteValue;
    //         {2'b11, 6'h01}: r_PhaseStep[w_VoiceOpRegWriteIndex][7:0] <= i_RegisterWriteValue;
    //         {2'b11, 6'h02}: r_Waveform[w_VoiceOpRegWriteIndex][15:8] <= i_RegisterWriteValue;
    //         {2'b11, 6'h03}: r_Waveform[w_VoiceOpRegWriteIndex][7:0] <= i_RegisterWriteValue;
    //         //
    //         {2'b11, 6'h04}: r_EnvelopeL1[w_VoiceOpRegWriteIndex] <= i_RegisterWriteValue;
    //         {2'b11, 6'h05}: r_EnvelopeL2[w_VoiceOpRegWriteIndex] <= i_RegisterWriteValue;
    //         {2'b11, 6'h06}: r_EnvelopeL3[w_VoiceOpRegWriteIndex] <= i_RegisterWriteValue;
    //         {2'b11, 6'h07}: r_EnvelopeL4[w_VoiceOpRegWriteIndex] <= i_RegisterWriteValue;
    //         {2'b11, 6'h08}: r_EnvelopeR1[w_VoiceOpRegWriteIndex] <= i_RegisterWriteValue;
    //         {2'b11, 6'h09}: r_EnvelopeR2[w_VoiceOpRegWriteIndex] <= i_RegisterWriteValue;
    //         {2'b11, 6'h0a}: r_EnvelopeR3[w_VoiceOpRegWriteIndex] <= i_RegisterWriteValue;
    //         {2'b11, 6'h0b}: r_EnvelopeR4[w_VoiceOpRegWriteIndex] <= i_RegisterWriteValue;
    //         //
    //         {2'b11, 6'h0c}: r_Algorithm[w_VoiceOpRegWriteIndex] <= i_RegisterWriteValue;
    //         //
    //         //
    //         {2'b10, 6'h00}: r_NoteOn[w_VoiceRegWriteIndex] <= i_RegisterWriteValue[7:0];
    //         // {2'b10, 6'h01}: <reserved>
    //         {2'b10, 6'h02}: r_CarrierComp[w_VoiceRegWriteIndex][15:8] <= i_RegisterWriteValue;  // TODO: Remove
    //         {2'b10, 6'h03}: r_CarrierComp[w_VoiceRegWriteIndex][7:0] <= i_RegisterWriteValue;  // TODO: Remove

    //         default: /* ignore writes to invalid registers */ ;
    //     endcase
    // end


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






// TODO
VoiceOperatorID_t w_OperatorWritebackID;
logic signed [15:0] w_OperatorWritebackValue;
assign w_OperatorWritebackID = 0;
assign w_OperatorWritebackValue = 0;




// TODO: Trim these down
VoiceOperatorID_t r_VoiceOperator [31:0];
AlgorithmWord_t r_AlgorithmWord [31:0];




logic signed [16:0] w_ModulatedPhase;
logic unsigned [15:0] w_RawPhase;


stage_phase_accumulation phase_accumulation (
    .i_Clock                     (i_Clock),
    .i_VoiceOperator             (r_VoiceOperator[0]),
    .o_VoiceOperator             (r_VoiceOperator[1]),

    .o_Phase                     (w_RawPhase),

    .i_PhaseStepConfigWriteEnable(w_PhaseStepWriteEnable),
    .i_PhaseStepConfigWriteAddr  (w_VoiceOpRegWriteIndex),
    .i_PhaseStepConfigWriteData  (i_RegisterWriteValue)
);



stage_modulation modulation (
    .i_Clock          (i_Clock),

    .i_VoiceOperator  (r_VoiceOperator[1]),
    .o_VoiceOperator  (r_VoiceOperator[2]),

    .i_Phase       (w_RawPhase),
    .o_Phase       (w_ModulatedPhase),


    .o_AlgorithmWord      (r_AlgorithmWord[2]),

    // TODO: Set these if register written above
    .i_AlgorithmWriteEnable(w_AlgorithmWriteEnable),
    .i_AlgorithmWriteAddr  (w_VoiceOpRegWriteIndex),
    .i_AlgorithmWriteData  (i_RegisterWriteValue[7:0]),

    .i_OperatorWritebackID   (w_OperatorWritebackID),
    .i_OperatorWritebackValue(w_OperatorWritebackValue)

);



logic signed [15:0] r_RawWaveform [32];

stage_waveform_generation waveform_generation (
    .i_Clock  (i_Clock),
    // .i_Phase   (w_ModulatedPhase),
    .i_Phase   ($signed({1'b0, w_RawPhase})),
    .o_Waveform(r_RawWaveform[3]),

    .i_VoiceOperator(r_VoiceOperator[2]),
    .o_VoiceOperator(r_VoiceOperator[3]),

    .i_AlgorithmWord(r_AlgorithmWord[2]),
    .o_AlgorithmWord(r_AlgorithmWord[3])
);


stage_sample_generator sample_generator (
    .i_Clock        (i_Clock),

    .i_VoiceOperator(r_VoiceOperator[3]),
    .i_AlgorithmWord (r_AlgorithmWord[3]),
    .i_OperatorOutput(r_RawWaveform[3]),

    .o_SampleReady  (o_SampleReady),
    .o_Sample        (o_Sample)
);



// TODO
// logic signed [15:0] r_Subsample;
// assign r_Subsample = r_RawWaveform[3];
// assign r_Subsample = w_ModulatedPhase[15:0];
// assign r_Subsample = w_RawPhase;
// logic r_SubsampleReady;
// assign r_SubsampleReady = r_VoiceOperator[3] == 8'hff;
// assign o_SampleReady = r_VoiceOperator[3] == 8'hff;


// logic signed [21:0] r_SampleBuffer;
// logic signed [21:0] w_SignExtendedSubsample;
// assign w_SignExtendedSubsample = {{6{r_Subsample[15]}}, r_Subsample};

// // Subsample combination and output
// always_ff @ (posedge i_Clock) begin

//     if (i_Reset || o_SampleReady)
//         r_SampleBuffer <= w_SignExtendedSubsample;
//     else if (r_SubsampleReady)
//         r_SampleBuffer <= r_SampleBuffer + w_SignExtendedSubsample;

//     if (o_SampleReady)
//         o_Sample <= r_SampleBuffer[20:5];

// end




endmodule
