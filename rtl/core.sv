
`include "core.svh"

// TODO: Remove
// verilator lint_off UNUSED

// TODO: Cleanup


module core (
    input logic i_Clock,
    input logic i_Reset,
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





// TODO: KeyOn, etc.
VoiceConfig_t w_VoiceConfig;
OperatorConfig_t w_OperatorConfig;
always_comb begin
    w_VoiceConfig = i_Config.VoiceConfigs[w_VoiceNum];
    w_OperatorConfig = w_VoiceConfig.OperatorConfigs[w_OperatorNum];
end



// Pipeline Registers
//
logic signed [15:0] r_M [16];
logic signed [15:0] r_F [16];
logic unsigned [15:0] r_Phase;
logic r_WaveformType;
logic [5:0] r_Algorithm [10];
logic signed [15:0] r_EnvelopeLevel;
logic signed [31:0] r_EnvelopeLevelProduct [3];  // TODO: ADSR
logic signed [31:0] r_WaveformAmplitudeProduct [3];
logic signed [15:0] r_WaveformAmplitude [11];
logic unsigned [6:0] r_CycleNumber [17];  // TODO: Do I really need this?
// I suspect it could be replaced with some subtraction and would save a few LUTs


logic signed [15:0] w_ModulationPhase;


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



typedef enum logic [2:0] {
    MUTE,
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE
} EnvelopeState_t;
EnvelopeState_t r_EnvelopeState [96];

// Calculate ADSR envelope
// TODO: Rename these registers
logic unsigned [16:0] r_L [96];
logic unsigned [7:0] r_AdsrClockCounter;
always_ff @ (posedge i_Clock) begin

    // TODO: Use the more advanced ADSR envelope shape from the DX7

    // r_L[r_CycleCounter] <= 'hffff;


    if (r_CycleCounter == 0) begin
        if (r_AdsrClockCounter[7])
            r_AdsrClockCounter <= 0;
        else
            r_AdsrClockCounter <= r_AdsrClockCounter + 1;
    end

    // TODO: Optimize this -- use a single bit if possible
    if (r_AdsrClockCounter[7]) begin
        // $display("Triggered on [%0d - %0d.%0d]", r_CycleCounter, w_VoiceNum, w_OperatorNum);

        case (r_EnvelopeState[r_CycleCounter])
            default: begin
                // Including MUTE
                r_L[r_CycleCounter] <= 0;
                if (w_VoiceConfig.KeyOn) begin
                    r_EnvelopeState[r_CycleCounter] <= ATTACK;
                    if (w_OperatorConfig.AttackLevel != 0) $display("[%0d - %0d.%0d] moving to ATTACK", r_CycleCounter, w_VoiceNum+1, w_OperatorNum+1);
                end
            end

            ATTACK: begin
                r_L[r_CycleCounter] <= r_L[r_CycleCounter] + w_OperatorConfig.AttackRate;
                if ( ! w_VoiceConfig.KeyOn)
                    r_EnvelopeState[r_CycleCounter] <= RELEASE;
                else if (r_L[r_CycleCounter] >= {1'b0, w_OperatorConfig.AttackLevel}) begin
                    r_EnvelopeState[r_CycleCounter] <= DECAY;
                    if (w_OperatorConfig.AttackRate != 0) $display("[%0d - %0d.%0d] (+%0d) moving to DECAY", r_CycleCounter, w_VoiceNum+1, w_OperatorNum+1, w_OperatorConfig.AttackRate);
                end
            end

            DECAY: begin
                r_L[r_CycleCounter] <= r_L[r_CycleCounter] - w_OperatorConfig.DecayRate;
                if ( ! w_VoiceConfig.KeyOn)
                    r_EnvelopeState[r_CycleCounter] <= RELEASE;
                else if (r_L[r_CycleCounter] <= {1'b0, w_OperatorConfig.SustainLevel}) begin
                    r_EnvelopeState[r_CycleCounter] <= SUSTAIN;
                    if (w_OperatorConfig.DecayRate != 0) $display("[%0d - %0d.%0d] (-%0d) moving to SUSTAIN", r_CycleCounter, w_VoiceNum+1, w_OperatorNum+1, w_OperatorConfig.DecayRate);
                end
            end

            SUSTAIN: begin
                r_L[r_CycleCounter] <= {1'b0, w_OperatorConfig.SustainLevel};
                if ( ! w_VoiceConfig.KeyOn)
                    r_EnvelopeState[r_CycleCounter] <= RELEASE;
            end

            RELEASE: begin
                r_L[r_CycleCounter] <= r_L[r_CycleCounter] - w_OperatorConfig.ReleaseRate;
                if (r_L[r_CycleCounter][16]) begin  // if L < 0
                    r_EnvelopeState[r_CycleCounter] <= MUTE;
                    if (w_OperatorConfig.ReleaseRate != 0) $display("[%0d - %0d.%0d] (-%0d) moving to MUTE", r_CycleCounter, w_VoiceNum+1, w_OperatorNum+1, w_OperatorConfig.ReleaseRate);
                end
            end
        endcase
    end
end





logic unsigned [6:0] r_CycleCounter;
logic unsigned [2:0] w_OperatorNum;
logic unsigned [3:0] w_VoiceNum;
assign w_OperatorNum = r_CycleCounter[6:4];
assign w_VoiceNum = r_CycleCounter[3:0];

logic unsigned [15:0] r_PhaseAcc [96];

always_ff @ (posedge i_Clock) begin
    // NOTE: With 8 operators, this wouldn't be necessary. The counter could just overflow.
    if (i_Reset || r_CycleCounter == 95)
        r_CycleCounter <= 0;
    else
        r_CycleCounter <= r_CycleCounter + 1;

    // Stage 1-2 (secondary branch): multi-carrier amplitude compensation (four clock cycles)
    // Note that the first line here really occurs at the same time as the other
    // Stage 1 loads. The multiplication starts immediately.
    // r_EnvelopeLevelProduct[0] <= w_OperatorConfig.AttackLevel * w_VoiceConfig.AmplitudeAdjust;
    r_EnvelopeLevelProduct[0] <= r_L[r_CycleCounter] * w_VoiceConfig.AmplitudeAdjust;
    r_EnvelopeLevelProduct[1] <= r_EnvelopeLevelProduct[0];
    r_EnvelopeLevelProduct[2] <= r_EnvelopeLevelProduct[1];
    r_EnvelopeLevel <= {r_EnvelopeLevelProduct[2][30:16], 1'b0};


    if (w_VoiceNum == 1 && w_OperatorNum == 5) begin
        // $display("r_L[%0d] = %d", r_CycleCounter, r_L[r_CycleCounter]);
    end


    // Stage 1: accumulate and modulate phase (1 cycle)
    r_PhaseAcc[r_CycleCounter] <= r_PhaseAcc[r_CycleCounter] + w_OperatorConfig.PhaseStep;
    r_Phase <= r_PhaseAcc[r_CycleCounter] + w_ModulationPhase;
    r_WaveformType <= w_OperatorConfig.Waveform;
    r_CycleNumber[0] <= r_CycleCounter;
    r_Algorithm[0] <= w_VoiceConfig.Algorithm;

    // Stage 2: waveform generation (3 cycles)
    //  (the magic is done in the waveform_generator module)
    r_CycleNumber[1] <= r_CycleNumber[0];
    r_CycleNumber[2] <= r_CycleNumber[1];
    r_CycleNumber[3] <= r_CycleNumber[2];
    r_Algorithm[1] <= r_Algorithm[0];
    r_Algorithm[2] <= r_Algorithm[1];
    r_Algorithm[3] <= r_Algorithm[2];

    // Stage 3: waveform amplitude (four clock cycles)
    r_WaveformAmplitudeProduct[0] <= w_RawWaveformAmplitude * r_EnvelopeLevel;
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
    r_Algorithm[4] <= r_Algorithm[3];
    r_Algorithm[5] <= r_Algorithm[4];
    r_Algorithm[6] <= r_Algorithm[5];

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
    r_Algorithm[7] <= r_Algorithm[6];
    r_Algorithm[8] <= r_Algorithm[7];
    r_Algorithm[9] <= r_Algorithm[8];

    // Stage 5: Algorithm ROM lookup (1 clock cycle)
    r_AlgorithmControlWord <= r_AlgorithmROM[{r_Algorithm[9], r_CycleNumber[14][6:4]}];
    r_CycleNumber[15] <= r_CycleNumber[14];
    r_WaveformAmplitude[8] <= r_WaveformAmplitude[7];

    // Reset M for the first operator
    if (r_CycleNumber[15][6:4] == 0) begin
        r_M[r_CycleNumber[15][3:0]] <= w_MREN ? r_WaveformAmplitude[8] : 0;
    end
    else if (w_MREN)  begin
        r_M[r_CycleNumber[15][3:0]] <= r_M[r_CycleNumber[15][3:0]] + r_WaveformAmplitude[8];
    end
    if (w_FREN) r_F[r_CycleNumber[15][3:0]] <= r_WaveformAmplitude[8];

    // If this is the last operator, send the output subsampler ready signal too.
    o_Subsample <= r_M[r_CycleNumber[15][3:0]] + r_WaveformAmplitude[8];
    o_SubsampleReady <= r_CycleNumber[15][6:4] == 5;
    o_SampleReady <= r_CycleNumber[15] == 95;

end


always_comb begin
    case (w_SEL)
        `SEL_NONE               : w_ModulationPhase = 0;
        `SEL_PREVIOUS           : w_ModulationPhase = r_WaveformAmplitude[8];
        `SEL_MREG               : w_ModulationPhase = r_M[r_CycleNumber[15][3:0]];
        `SEL_MREG_PLUS_PREVIOUS : w_ModulationPhase = r_M[r_CycleNumber[15][3:0]] + r_WaveformAmplitude[8];
        // `SEL_FREG               : w_ModulationPhase = r_F[r_CycleNumber[15][3:0]];
        default                 : w_ModulationPhase = 0;
    endcase
end


endmodule
