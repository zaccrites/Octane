
`include "core.svh"

// TODO: Remove
// verilator lint_off UNUSED


module core (
    input logic i_Clock,
    input CoreConfig_t i_Config,

    output logic signed [15:0] o_Subsample,
    output logic o_SubsampleReady,
    output logic o_SampleReady
);



logic signed [15:0] w_RawWaveformAmplitude;
waveform_generator wavegen (
    .i_Clock    (i_Clock),
    .i_Phase    (r_Phase[15:3]),
    .i_Waveform (r_WaveformType),
    .o_Amplitude(w_RawWaveformAmplitude)
);



logic unsigned [6:0] r_CycleCounter;
logic unsigned [2:0] w_OperatorNum;
logic unsigned [3:0] w_VoiceNum;
assign w_OperatorNum = r_CycleCounter[6:4];
assign w_VoiceNum = r_CycleCounter[3:0];

logic unsigned [15:0] r_PhaseAcc [96];


// TODO: KeyOn, etc.
VoiceConfig_t w_VoiceConfig;
OperatorConfig_t w_OperatorConfig;
always_comb begin
    // o_SubsampleReady = w_OperatorNum == 5;
    // o_SampleReady = o_SubsampleReady && (w_VoiceNum == 15);

    w_VoiceConfig = i_Config.VoiceConfigs[w_VoiceNum];
    w_OperatorConfig = w_VoiceConfig.OperatorConfigs[w_OperatorNum];
end



// Pipeline Registers
//
logic unsigned [15:0] r_Phase;
//
logic r_WaveformType;
//
logic signed [15:0] r_EnvelopeLevel [4];
//
logic signed [31:0] r_WaveformAmplitudeProduct [3];  // TODO: ADSR and COM
//
logic signed [15:0] r_WaveformAmplitude [11];
//
logic unsigned [6:0] r_CycleNumber [16];  // TODO: Do I really need this?
// I suspect it could be replaced with some subtraction and would save a few LUTs


// Algorithm Control
// -------------------------------------------------------------
logic [4:0] r_AlgorithmROM [512];
initial $readmemh("roms/algorithm_rom.hex", r_AlgorithmROM);
logic [4:0] r_AlgorithmControlWord;

logic [2:0] w_SEL;
logic w_MREN;
logic w_FREN;
assign w_SEL  = r_AlgorithmControlWord[4:2];
assign w_MREN = r_AlgorithmControlWord[1];
assign w_FREN = r_AlgorithmControlWord[0];
// -------------------------------------------------------------



logic signed [15:0] r_M [16];
logic signed [15:0] r_ModulationPhase;

// TODO
// verilator lint_off UNUSED
logic signed [15:0] r_F [16];
// verilator lint_on UNUSED



always_ff @ (posedge i_Clock) begin
    // NOTE: With 8 operators, this wouldn't be necessary. The counter could just overflow.
    if (r_CycleCounter == 95)
        r_CycleCounter <= 0;
    else
        r_CycleCounter <= r_CycleCounter + 1;

    // Stage 1: accumulate and modulate phase (1 cycle)
    r_PhaseAcc[r_CycleCounter] <= r_PhaseAcc[r_CycleCounter] + w_OperatorConfig.PhaseStep;
    r_Phase <= r_PhaseAcc[r_CycleCounter];
    // r_Phase <= r_PhaseAcc[r_CycleCounter] + r_ModulationPhase;
    r_EnvelopeLevel[0] <= w_OperatorConfig.EnvelopeLevel;
    r_WaveformType <= w_OperatorConfig.Waveform;
    r_CycleNumber[0] <= r_CycleCounter;

    // if (r_CycleNumber[0] == 0) $display("phase = %d", r_Phase);

    // Stage 2: waveform generation (3 cycles)
    //  (the magic is done in the waveform_generator module)
    r_EnvelopeLevel[1] <= r_EnvelopeLevel[0];
    r_EnvelopeLevel[2] <= r_EnvelopeLevel[1];
    r_EnvelopeLevel[3] <= r_EnvelopeLevel[2];
    r_CycleNumber[1] <= r_CycleNumber[0];
    r_CycleNumber[2] <= r_CycleNumber[1];
    r_CycleNumber[3] <= r_CycleNumber[2];

    // Stage 3: waveform amplitude (four clock cycles)
    // if (r_CycleNumber[3] == 0) $display("raw = %d", w_RawWaveformAmplitude);

    r_WaveformAmplitudeProduct[0] <= w_RawWaveformAmplitude * r_EnvelopeLevel[3];
    r_WaveformAmplitudeProduct[1] <= r_WaveformAmplitudeProduct[0];
    r_WaveformAmplitudeProduct[2] <= r_WaveformAmplitudeProduct[1];
    r_WaveformAmplitude[0] <= {r_WaveformAmplitudeProduct[2][30:16], 1'b0};
    // The output is only half magnitude, so shift left by 1 to get it back.
    // TODO: Is it worth doing this? We lose one bit of information, and will
    // be dividing before combining voices or carrier operators anyway.
    r_CycleNumber[4] <= r_CycleNumber[3];
    r_CycleNumber[5] <= r_CycleNumber[4];
    r_CycleNumber[6] <= r_CycleNumber[5];
    r_CycleNumber[7] <= r_CycleNumber[6];

    // if (r_CycleNumber[7] == 0) $display("amp = %d", r_WaveformAmplitude[0]);


    // Stage 4: stall to provide the sample just-in-time to modulate the next operator
    r_WaveformAmplitude[1] <= r_WaveformAmplitude[0];
    r_WaveformAmplitude[2] <= r_WaveformAmplitude[1];
    r_WaveformAmplitude[3] <= r_WaveformAmplitude[2];
    r_WaveformAmplitude[4] <= r_WaveformAmplitude[3];
    r_WaveformAmplitude[5] <= r_WaveformAmplitude[4];
    r_WaveformAmplitude[6] <= r_WaveformAmplitude[5];
    r_WaveformAmplitude[7] <= r_WaveformAmplitude[6];
    r_CycleNumber[8] <= r_CycleNumber[7];
    r_CycleNumber[9] <= r_CycleNumber[8];
    r_CycleNumber[10] <= r_CycleNumber[9];
    r_CycleNumber[11] <= r_CycleNumber[10];
    r_CycleNumber[12] <= r_CycleNumber[11];
    r_CycleNumber[13] <= r_CycleNumber[12];
    r_CycleNumber[14] <= r_CycleNumber[13];

    // Stage 5: Algorithm ROM lookup (2 clock cycles)
    // Determine which operator was activate at the start of the pipeline
    // by subtracting a constant number of clock cycles from the counter.
    //
    // TODO: Use algorithm directly here is likely a bug, since the voice
    // won't be the same as the currently active one.
    // I may have to pipeline the algorithm number.
    //
    // Another option is to do this calculation before stalling, then
    // just pipeline the computed value and the control signals (and e.g. the index into r_M and r_F)
    //
    // r_AlgorithmRomIndex <= {};
    r_AlgorithmControlWord <= r_AlgorithmROM[{w_VoiceConfig.Algorithm, r_CycleNumber[14][6:4]}];
    r_CycleNumber[15] <= r_CycleNumber[14];
    r_WaveformAmplitude[8] <= r_WaveformAmplitude[7];
    r_WaveformAmplitude[9] <= r_WaveformAmplitude[8];

    // Stage 6: Latch result into Selector, M, and F registers.
    case (w_SEL)
        `SEL_NONE               : r_ModulationPhase <= 0;
        `SEL_PREVIOUS           : r_ModulationPhase <= r_WaveformAmplitude[9];
        `SEL_MREG               : r_ModulationPhase <= r_M[r_CycleNumber[15][3:0]];
        `SEL_MREG_PLUS_PREVIOUS : r_ModulationPhase <= r_M[r_CycleNumber[15][3:0]] + r_WaveformAmplitude[9];
        // `SEL_FREG               : r_ModulationPhase <= r_F[r_CycleNumber[15][3:0]];
        default                 : r_ModulationPhase <= 0;
    endcase
    if (w_MREN) r_M[r_CycleNumber[15][3:0]] <= r_M[r_CycleNumber[15][3:0]] + r_WaveformAmplitude[9];
    if (w_FREN) r_F[r_CycleNumber[15][3:0]] <= r_WaveformAmplitude[9];
    //
    // If this is the last operator, send the output subsampler ready signal too.
    o_Subsample <= r_WaveformAmplitude[9];
    o_SubsampleReady <= r_CycleNumber[15][6:4] == 5;
    o_SampleReady <= r_CycleNumber[15] == 95;

    if (r_CycleNumber[15] == 0) $display("amp = %d", r_WaveformAmplitude[9]);



end


endmodule
