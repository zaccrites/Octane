
`include "core.svh"


// TODO
// verilator lint_off UNUSED


module core (
    input logic i_Clock,
    // input logic i_Reset,

    input CoreConfig_t i_Config,

    // We output a subsample after performing the algorithm for each
    // operator for each voice. The complete sample is ready when the final
    // subsample is output.
    output logic signed [15:0] o_Subsample,
    output logic signed o_SubsampleReady,
    output logic o_SampleReady
);


// Global registers
logic unsigned [6:0] r_CycleCounter;
logic unsigned [2:0] w_OperatorNum;
logic unsigned [3:0] w_VoiceNum;
assign w_OperatorNum = r_CycleCounter[6:4];
assign w_VoiceNum = r_CycleCounter[3:0];


// Individual phase accumulators for each operator for each voice
logic unsigned [15:0] r_PhaseAcc [96];
logic unsigned [15:0] r_Phase;



logic signed [15:0] w_RawWaveformAmplitude;
waveform_generator wavegen (
    .i_Clock    (i_Clock),
    .i_Phase    (r_Phase[15:3]),
    .i_Waveform (r_WaveformType[1]),
    .o_Amplitude(w_RawWaveformAmplitude)
);


VoiceConfig_t w_VoiceConfig;
OperatorConfig_t w_OperatorConfig;
always_comb begin
    o_SubsampleReady = w_OperatorNum == 5;
    o_SampleReady = o_SubsampleReady && (w_VoiceNum == 15);

    w_VoiceConfig = i_Config.VoiceConfigs[w_VoiceNum];
    w_OperatorConfig = w_VoiceConfig.OperatorConfigs[w_OperatorNum];
end


logic r_WaveformType [2];
logic signed [15:0] r_EnvelopeLevel [5];
logic signed [31:0] r_WaveformAmplitude [3];  // TODO: ADSR, and COM as well


// TODO: Operator sample "sub-sub sample"?
logic signed [15:0] r_Subsample [8];




logic [2:0] w_SEL;
logic w_MREN;
logic w_FREN;
algorithm_rom control (
    .i_Clock    (i_Clock),
    .i_Algorithm(w_VoiceConfig.Algorithm),
    .i_Operator (w_OperatorNum),  // TODO: This may need to be offset or pipelined somehow
    .o_SEL      (w_SEL),
    .o_MREN     (w_MREN),
    .o_FREN     (w_FREN)
);

logic signed [15:0] r_M [16];
logic signed [15:0] r_F [16];
logic signed [15:0] r_Previous [16];
logic signed [15:0] w_ModulationPhase;

always_ff @ (posedge i_Clock) begin
    if (w_OperatorNum == 0) begin
        // If this is the first operator, reset M (optionally to the new result)
        r_M[w_VoiceNum] <= w_MREN ? r_Subsample[7] : 0;
    end
    else if (w_MREN) begin
        r_M[w_VoiceNum] <= r_M[w_VoiceNum] + r_Subsample[7];
    end

    if (w_FREN) r_F[w_VoiceNum] <= r_F[w_VoiceNum] + r_Subsample[7];

    r_Previous[w_VoiceNum] <= r_Subsample[7];

end

always_comb begin
    case (w_SEL)
        `SEL_NONE               : w_ModulationPhase = 0;
        `SEL_PREVIOUS           : w_ModulationPhase = r_Previous[w_VoiceNum];
        `SEL_MREG               : w_ModulationPhase = r_M[w_VoiceNum];
        `SEL_MREG_PLUS_PREVIOUS : w_ModulationPhase = r_Previous[w_VoiceNum] + r_M[w_VoiceNum];
        `SEL_FREG               : w_ModulationPhase = r_F[w_VoiceNum];
        default                 : w_ModulationPhase = 0;
    endcase
end


always_ff @ (posedge i_Clock) begin
    // NOTE: With 8 operators, this wouldn't be necessary. The counter could just overflow.
    if (w_OperatorNum == 5 && w_VoiceNum == 15)
        r_CycleCounter <= 0;
    else
        r_CycleCounter <= r_CycleCounter + 1;

    // Stage 1: Read config and phase accumulation (one clock cycle)
    r_PhaseAcc[r_CycleCounter] <= r_PhaseAcc[r_CycleCounter] + w_OperatorConfig.PhaseStep;
    r_EnvelopeLevel[0] <= w_OperatorConfig.EnvelopeLevel;
    r_WaveformType[0] <= w_OperatorConfig.Waveform;

    // Stage 2: Modulation, including feedback (one clock cycle)
    r_Phase <= r_PhaseAcc[r_CycleCounter] + w_ModulationPhase;
    r_EnvelopeLevel[1] <= r_EnvelopeLevel[0];
    r_WaveformType[1] <= r_WaveformType[0];

    // Stage 3: waveform generation via above module (three clock cycles)
    r_EnvelopeLevel[2] <= r_EnvelopeLevel[1];
    r_EnvelopeLevel[3] <= r_EnvelopeLevel[2];
    r_EnvelopeLevel[4] <= r_EnvelopeLevel[3];

    // Stage 4: waveform amplitude envelope (four clock cycles)
    r_WaveformAmplitude[0] <= w_RawWaveformAmplitude * r_EnvelopeLevel[4];
    r_WaveformAmplitude[1] <= r_WaveformAmplitude[0];
    r_WaveformAmplitude[2] <= r_WaveformAmplitude[1];
    // The output is only half magnitude, so shift left by 1 to get it back.
    // TODO: Is it worth doing this? We lose one bit of information, and will
    // be dividing before combining voices or carrier operators anyway.
    r_Subsample[0] <= {r_WaveformAmplitude[2][30:16], 1'b0};
    // not really the subsample-- likely to be written to M or F, or used immediately by Selector

    // Stage 5: stall until we get to 16 clocks total (seven clock cycles)
    r_Subsample[1] <= r_Subsample[0];
    r_Subsample[2] <= r_Subsample[1];
    r_Subsample[3] <= r_Subsample[2];
    r_Subsample[4] <= r_Subsample[3];
    r_Subsample[5] <= r_Subsample[4];
    r_Subsample[6] <= r_Subsample[5];
    r_Subsample[7] <= r_Subsample[6];
    // Subsample is only valid once the algorithm is finished for each voice

end

assign o_Subsample = r_Subsample[7];



endmodule
