
`include "synth.svh"


module stage_envelope_attenuator (
    input i_Clock,

    input VoiceOperatorID_t i_VoiceOperator,
    output VoiceOperatorID_t o_VoiceOperator,

    input AlgorithmWord_t i_AlgorithmWord,
    output AlgorithmWord_t o_AlgorithmWord,

    input signed [15:0] i_Waveform,
    output signed [15:0] o_Waveform,

    input logic [7:0] i_EnvelopeConfigWriteEnable,
    input logic [3:0] i_NoteOnConfigWriteEnable,
    input VoiceOperatorID_t i_ConfigWriteAddr,
    input logic unsigned [7:0] i_ConfigWriteData
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

typedef logic unsigned [7:0] EnvelopeLevelConfig_t;
typedef logic unsigned [7:0] EnvelopeRateConfig_t;

EnvelopeLevelConfig_t r_EnvelopeLevelConfig [4] [`NUM_VOICE_OPERATORS];
EnvelopeRateConfig_t r_EnvelopeRateConfig [4] [`NUM_VOICE_OPERATORS];


typedef enum logic [2:0] {
    MUTE,
    ATTACK,
    DECAY,
    RECOVERY,
    SUSTAIN,
    RELEASE
} EnvelopeRegion_t;

typedef logic unsigned [12:0] EnvelopeLevel_t;

typedef struct packed {
    // NOTE: Must be 16 bits or less to fit into a block RAM!
    EnvelopeRegion_t Region;
    EnvelopeLevel_t Level;
} EnvelopeState_t;

EnvelopeState_t r_EnvelopeStates [`NUM_VOICE_OPERATORS];

`define LEVEL_MAX  13'h1fff

function EnvelopeLevel_t increaseLevel(EnvelopeLevel_t level, EnvelopeRateConfig_t rate);
    if (level > `LEVEL_MAX - extendRateConfig(rate))
        return `LEVEL_MAX;
    else
        return level + extendRateConfig(rate);
endfunction

function EnvelopeLevel_t decreaseLevel(EnvelopeLevel_t level, EnvelopeRateConfig_t rate);
    if (extendRateConfig(rate) > level)
        return 0;
    else
        return level - extendRateConfig(rate);
endfunction

function EnvelopeLevel_t extendLevelConfig(EnvelopeLevelConfig_t levelConfig);
    // To make envelopes last even longer, levels are made larger...
    return {levelConfig, 5'b0};
endfunction

function EnvelopeLevel_t extendRateConfig(EnvelopeRateConfig_t rateConfig);
    // ... and rates are made smaller.
    return {5'b0, rateConfig};
endfunction


VoiceOperatorID_t r_VoiceOperator [4];
AlgorithmWord_t r_AlgorithmWord [4];


logic r_DoEnvelopCalc;
EnvelopeState_t r_EnvelopeState;
logic r_NoteOn;
EnvelopeRateConfig_t r_EnvelopeRate [4];
EnvelopeLevelConfig_t r_EnvelopeLevelTarget [4];


logic signed [15:0] r_RawWaveform;
logic signed [31:0] r_AttenuatedWaveformProduct [3];


always_ff @ (posedge i_Clock) begin

    if (i_NoteOnConfigWriteEnable[3])
        r_NoteOnConfig[31:24] <= i_ConfigWriteData;
    if (i_NoteOnConfigWriteEnable[2])
        r_NoteOnConfig[23:16] <= i_ConfigWriteData;
    if (i_NoteOnConfigWriteEnable[1])
        r_NoteOnConfig[15:8] <= i_ConfigWriteData;
    if (i_NoteOnConfigWriteEnable[0])
        r_NoteOnConfig[7:0] <= i_ConfigWriteData;

    // TODO: Ensure that two of these parameters are shared by each of four block RAMs.
    if (i_EnvelopeConfigWriteEnable[0])
        r_EnvelopeLevelConfig[0][i_ConfigWriteAddr] <= i_ConfigWriteData;
    if (i_EnvelopeConfigWriteEnable[1])
        r_EnvelopeLevelConfig[1][i_ConfigWriteAddr] <= i_ConfigWriteData;
    if (i_EnvelopeConfigWriteEnable[2])
        r_EnvelopeLevelConfig[2][i_ConfigWriteAddr] <= i_ConfigWriteData;
    if (i_EnvelopeConfigWriteEnable[3])
        r_EnvelopeLevelConfig[3][i_ConfigWriteAddr] <= i_ConfigWriteData;

    if (i_EnvelopeConfigWriteEnable[4])
        r_EnvelopeRateConfig[0][i_ConfigWriteAddr] <= i_ConfigWriteData;
    if (i_EnvelopeConfigWriteEnable[5])
        r_EnvelopeRateConfig[1][i_ConfigWriteAddr] <= i_ConfigWriteData;
    if (i_EnvelopeConfigWriteEnable[6])
        r_EnvelopeRateConfig[2][i_ConfigWriteAddr] <= i_ConfigWriteData;
    if (i_EnvelopeConfigWriteEnable[7])
        r_EnvelopeRateConfig[3][i_ConfigWriteAddr] <= i_ConfigWriteData;

    // Clock 1
    // ----------------------------------------------------------

    r_DoEnvelopCalc <= w_DoEnvelopeCalc;
    r_EnvelopeState <= r_EnvelopeStates[i_VoiceOperator];
    r_NoteOn <= r_NoteOnConfig[getVoiceID(i_VoiceOperator)];
    r_RawWaveform <= i_Waveform;

    // TODO: Rename these? They are only pipelined once, but there are four of each.
    r_EnvelopeRate[0] <= r_EnvelopeRateConfig[0][i_VoiceOperator];
    r_EnvelopeRate[1] <= r_EnvelopeRateConfig[1][i_VoiceOperator];
    r_EnvelopeRate[2] <= r_EnvelopeRateConfig[2][i_VoiceOperator];
    r_EnvelopeRate[3] <= r_EnvelopeRateConfig[3][i_VoiceOperator];

    r_EnvelopeLevelTarget[0] <= r_EnvelopeLevelConfig[0][i_VoiceOperator];
    r_EnvelopeLevelTarget[1] <= r_EnvelopeLevelConfig[1][i_VoiceOperator];
    r_EnvelopeLevelTarget[2] <= r_EnvelopeLevelConfig[2][i_VoiceOperator];
    r_EnvelopeLevelTarget[3] <= r_EnvelopeLevelConfig[3][i_VoiceOperator];

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
                r_EnvelopeStates[r_VoiceOperator[0]].Level <= increaseLevel(r_EnvelopeState.Level, r_EnvelopeRate[0]);
            end

            if ( ! r_NoteOn) begin
                r_EnvelopeStates[r_VoiceOperator[0]].Region <= RELEASE;
            end
            else if (r_EnvelopeState.Level >= extendLevelConfig(r_EnvelopeLevelTarget[0])) begin
                r_EnvelopeStates[r_VoiceOperator[0]].Region <= DECAY;
            end
        end

        DECAY: begin
            if (r_DoEnvelopCalc) begin
                r_EnvelopeStates[r_VoiceOperator[0]].Level <= decreaseLevel(r_EnvelopeState.Level, r_EnvelopeRate[1]);
            end

            if ( ! r_NoteOn) begin
                r_EnvelopeStates[r_VoiceOperator[0]].Region <= RELEASE;
            end
            else if (r_EnvelopeState.Level <= extendLevelConfig(r_EnvelopeLevelTarget[1])) begin
                r_EnvelopeStates[r_VoiceOperator[0]].Region <= RECOVERY;
            end
        end

        RECOVERY: begin
            // FUTURE: Do I need to handle a situation where L2 > L3?
            // I'll need to subtract from L in that case, instead of add.
            // Many DX7 envelope diagrams show this case.
            // http://www.audiocentralmagazine.com/wp-content/uploads/2012/04/dx7-envelope.png

            if (r_DoEnvelopCalc) begin
                r_EnvelopeStates[r_VoiceOperator[0]].Level <= increaseLevel(r_EnvelopeState.Level, r_EnvelopeRate[2]);
            end

            if ( ! r_NoteOn) begin
                r_EnvelopeStates[r_VoiceOperator[0]].Region <= RELEASE;
            end
            else if (r_EnvelopeState.Level >= extendLevelConfig(r_EnvelopeLevelTarget[2])) begin
                r_EnvelopeStates[r_VoiceOperator[0]].Region <= SUSTAIN;
            end
        end

        SUSTAIN: begin
            r_EnvelopeStates[r_VoiceOperator[0]].Level <= extendLevelConfig(r_EnvelopeLevelTarget[2]);
            if ( ! r_NoteOn) begin
                r_EnvelopeStates[r_VoiceOperator[0]].Region <= RELEASE;
            end
        end

        RELEASE: begin
            if (r_DoEnvelopCalc) begin
                r_EnvelopeStates[r_VoiceOperator[0]].Level <= decreaseLevel(r_EnvelopeState.Level, r_EnvelopeRate[3]);
            end
            if (r_EnvelopeState.Level <= extendLevelConfig(r_EnvelopeLevelTarget[3])) begin
                r_EnvelopeStates[r_VoiceOperator[0]].Region <= MUTE;
            end
        end

    endcase

    // ----------------------------------------------------------

    // Clocks 2-5 : Attenuate output sample
    // ----------------------------------------------------------

    r_AttenuatedWaveformProduct[0] <= r_RawWaveform * $signed({1'b0, r_EnvelopeState.Level, 3'b00});
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
