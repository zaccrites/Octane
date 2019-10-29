
`include "synth.svh"


module stage_envelope_attenuator (
    input i_Clock,

    input VoiceOperatorID_t i_VoiceOperator,
    output VoiceOperatorID_t o_VoiceOperator,

    input AlgorithmWord_t i_AlgorithmWord,
    output AlgorithmWord_t o_AlgorithmWord,

    input signed [15:0] i_Waveform,
    output signed [15:0] o_Waveform,

    input logic [4:0] i_EnvelopeConfigWriteEnable,
    input logic [1:0] i_NoteOnConfigWriteEnable,
    input VoiceOperatorID_t i_ConfigWriteAddr,
    input logic [15:0] i_ConfigWriteData
);


// At the start of every 256 cycles, we increment a counter.
// One bit of this counter is used to divide the main clock
// for envelope calculation.
logic unsigned [7:0] r_ClockCounter;
logic w_DoEnvelopeCalc;
always_comb w_DoEnvelopeCalc = r_ClockCounter[4];

always_ff @ (posedge i_Clock) begin
    if (i_VoiceOperator == 0) begin
        if (w_DoEnvelopeCalc)
            r_ClockCounter <= 0;
        else
            r_ClockCounter <= r_ClockCounter + 1;
    end
end


/// Stored per-voice
logic [31:0] r_NoteOnConfig;

typedef logic unsigned [13:0] EnvelopeLevel_t;
typedef logic unsigned [11:0] EnvelopeRate_t;

// TODO: These add up to 64 bits, so could be stored in
// four BRAMs if one of the fields is suitably cut up.
EnvelopeLevel_t r_AttackLevelConfig [`NUM_VOICE_OPERATORS];
EnvelopeLevel_t r_SustainLevelConfig [`NUM_VOICE_OPERATORS];
EnvelopeRate_t r_AttackRateConfig [`NUM_VOICE_OPERATORS];
EnvelopeRate_t r_DecayRateConfig [`NUM_VOICE_OPERATORS];
EnvelopeRate_t r_ReleaseRateConfig [`NUM_VOICE_OPERATORS];


typedef enum logic [2:0] {
    MUTE,
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE
} EnvelopeRegion_t;

typedef struct packed {
    // NOTE: Must be 16 bits or less to fit into a block RAM!
    // TODO: Need to solve this, since levels are 14 bits now.
    // I may have to back it off to 13 bits.
    EnvelopeRegion_t Region;
    EnvelopeLevel_t Level;
} EnvelopeState_t;

EnvelopeState_t r_EnvelopeStates [`NUM_VOICE_OPERATORS];

`define LEVEL_MAX  14'h3fff

function EnvelopeLevel_t increaseLevel(EnvelopeLevel_t level, EnvelopeRate_t rate);
    if (level > `LEVEL_MAX - extendRateConfig(rate))
        return `LEVEL_MAX;
    else
        return level + extendRateConfig(rate);
endfunction

function EnvelopeLevel_t decreaseLevel(EnvelopeLevel_t level, EnvelopeRate_t rate);
    if (extendRateConfig(rate) > level)
        return 0;
    else
        return level - extendRateConfig(rate);
endfunction

function EnvelopeLevel_t extendRateConfig(EnvelopeRate_t rateConfig);
    return {2'b0, rateConfig};
endfunction


VoiceOperatorID_t r_VoiceOperator [4];
AlgorithmWord_t r_AlgorithmWord [4];


logic r_DoEnvelopCalc;
EnvelopeState_t r_EnvelopeState;
logic r_NoteOn;

EnvelopeLevel_t r_AttackLevel;
EnvelopeLevel_t r_SustainLevel;
EnvelopeRate_t r_AttackRate;
EnvelopeRate_t r_DecayRate;
EnvelopeRate_t r_ReleaseRate;

logic signed [15:0] r_RawWaveform;
logic signed [31:0] r_AttenuatedWaveformProduct [3];


always_ff @ (posedge i_Clock) begin

    if (i_NoteOnConfigWriteEnable[1]) r_NoteOnConfig[31:16] <= i_ConfigWriteData;
    if (i_NoteOnConfigWriteEnable[0]) r_NoteOnConfig[15:0] <= i_ConfigWriteData;

    // TODO: Ensure that two of these parameters are shared by each of four block RAMs.
    if (i_EnvelopeConfigWriteEnable[0]) r_AttackLevelConfig[i_ConfigWriteAddr] <= i_ConfigWriteData[13:0];
    if (i_EnvelopeConfigWriteEnable[1]) r_SustainLevelConfig[i_ConfigWriteAddr] <= i_ConfigWriteData[13:0];
    if (i_EnvelopeConfigWriteEnable[2]) r_AttackRateConfig[i_ConfigWriteAddr] <= i_ConfigWriteData[11:0];
    if (i_EnvelopeConfigWriteEnable[3]) r_DecayRateConfig[i_ConfigWriteAddr] <= i_ConfigWriteData[11:0];
    if (i_EnvelopeConfigWriteEnable[4]) r_ReleaseRateConfig[i_ConfigWriteAddr] <= i_ConfigWriteData[11:0];

    // Clock 1
    // ----------------------------------------------------------

    r_DoEnvelopCalc <= w_DoEnvelopeCalc;
    r_EnvelopeState <= r_EnvelopeStates[i_VoiceOperator];
    r_NoteOn <= r_NoteOnConfig[getVoiceID(i_VoiceOperator)];
    r_RawWaveform <= i_Waveform;

    r_AttackLevel <= r_AttackLevelConfig[i_VoiceOperator];
    r_SustainLevel <= r_SustainLevelConfig[i_VoiceOperator];
    r_AttackRate <= r_AttackRateConfig[i_VoiceOperator];
    r_DecayRate <= r_DecayRateConfig[i_VoiceOperator];
    r_ReleaseRate <= r_ReleaseRateConfig[i_VoiceOperator];

    r_AlgorithmWord[0] <= i_AlgorithmWord;
    r_VoiceOperator[0] <= i_VoiceOperator;

    // ----------------------------------------------------------

    // Clock 2 : Update state machine (separate from main pipeline)
    // ----------------------------------------------------------

    // TODO: There is a lot of combinational logic in this stage.
    // If it is a critical path then it may make sense to
    // split up into one stage for each arm of the state machine.

    case (r_EnvelopeState.Region)

        // MUTE:
        default: begin
            r_EnvelopeStates[r_VoiceOperator[0]].Level <= 0;
            if (r_NoteOn) begin

                r_EnvelopeStates[r_VoiceOperator[0]].Region <= ATTACK;
            end
        end

        ATTACK: begin
            if (r_DoEnvelopCalc) begin
                r_EnvelopeStates[r_VoiceOperator[0]].Level <= increaseLevel(r_EnvelopeState.Level, r_AttackRate);
            end

            if ( ! r_NoteOn) begin
                r_EnvelopeStates[r_VoiceOperator[0]].Region <= RELEASE;
            end
            else if (r_EnvelopeState.Level >= r_AttackLevel) begin
                r_EnvelopeStates[r_VoiceOperator[0]].Region <= DECAY;
            end
        end

        DECAY: begin
            if (r_DoEnvelopCalc) begin
                r_EnvelopeStates[r_VoiceOperator[0]].Level <= decreaseLevel(r_EnvelopeState.Level, r_DecayRate);
            end

            if ( ! r_NoteOn) begin
                r_EnvelopeStates[r_VoiceOperator[0]].Region <= RELEASE;
            end
            else if (r_EnvelopeState.Level <= r_SustainLevel) begin
                r_EnvelopeStates[r_VoiceOperator[0]].Region <= SUSTAIN;
            end
        end

        SUSTAIN: begin
            r_EnvelopeStates[r_VoiceOperator[0]].Level <= r_SustainLevel;
            if ( ! r_NoteOn) begin
                r_EnvelopeStates[r_VoiceOperator[0]].Region <= RELEASE;
            end
        end

        RELEASE: begin
            if (r_DoEnvelopCalc) begin
                r_EnvelopeStates[r_VoiceOperator[0]].Level <= decreaseLevel(r_EnvelopeState.Level, r_ReleaseRate);
            end
            if (r_EnvelopeState.Level == 0) begin
                r_EnvelopeStates[r_VoiceOperator[0]].Region <= MUTE;
            end
        end

    endcase

    // ----------------------------------------------------------

    // Clocks 2-5 : Attenuate output sample
    // ----------------------------------------------------------

    // r_AttenuatedWaveformProduct[0] <= r_RawWaveform * $signed({1'b0, r_EnvelopeState.Level, 1'b0});
    r_AttenuatedWaveformProduct[0] <= r_RawWaveform * 16'h7fff;
    r_AttenuatedWaveformProduct[1] <= r_AttenuatedWaveformProduct[0];
    r_AttenuatedWaveformProduct[2] <= r_AttenuatedWaveformProduct[1];
    o_Waveform <= r_AttenuatedWaveformProduct[2][31:16];

    r_VoiceOperator[1] <= r_VoiceOperator[0];
    r_VoiceOperator[2] <= r_VoiceOperator[1];
    r_VoiceOperator[3] <= r_VoiceOperator[2];
    o_VoiceOperator <= r_VoiceOperator[3];

    r_AlgorithmWord[1] <= r_AlgorithmWord[0];
    r_AlgorithmWord[2] <= r_AlgorithmWord[1];
    r_AlgorithmWord[3] <= r_AlgorithmWord[2];
    o_AlgorithmWord <= r_AlgorithmWord[3];

    // ----------------------------------------------------------

end


endmodule
