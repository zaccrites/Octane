
`include "synth.svh"


module stage_envelope_attenuator (
    input i_Clock,

    input logic `VOICE_OPERATOR_ID i_VoiceOperator,
    output logic `VOICE_OPERATOR_ID o_VoiceOperator,

    input logic i_NoteOn,

    input logic `ALGORITHM_WORD i_AlgorithmWord,
    output logic `ALGORITHM_WORD o_AlgorithmWord,

    input logic signed [15:0] i_Waveform,
    output logic signed [15:0] o_Waveform,

    input logic [4:0] i_EnvelopeConfigWriteEnable,
    input logic `VOICE_OPERATOR_ID i_ConfigWriteAddr,
    //
    // Not all of the config data bits are used
    // verilator lint_off UNUSED
    input logic [15:0] i_ConfigWriteData
    // verilator lint_on UNUSED
);


// At the start of every 256 cycles, we increment a counter.
// One bit of this counter is used to divide the main clock
// for envelope calculation.
logic [7:0] r_ClockCounter;
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


`define ENVELOPE_LEVEL [13:0]
`define ENVELOPE_RATE  [11:0]


// TODO: These add up to 64 bits, so could be stored in
// four BRAMs if one of the fields is suitably cut up.
logic `ENVELOPE_LEVEL r_AttackLevelConfig  [`NUM_VOICE_OPERATORS];
logic `ENVELOPE_LEVEL r_SustainLevelConfig [`NUM_VOICE_OPERATORS];
logic `ENVELOPE_RATE  r_AttackRateConfig   [`NUM_VOICE_OPERATORS];
logic `ENVELOPE_RATE  r_DecayRateConfig    [`NUM_VOICE_OPERATORS];
logic `ENVELOPE_RATE  r_ReleaseRateConfig  [`NUM_VOICE_OPERATORS];


`define ENVELOPE_STATE  [2:0]
`define MUTE     3'd0
`define ATTACK   3'd1
`define DECAY    3'd2
`define SUSTAIN  3'd3
`define RELEASE  3'd4

// NOTE: Must be 16 bits or less to fit into a block RAM!
// TODO: Need to solve this, since levels are 14 bits now.
// I may have to back it off to 13 bits.
logic `ENVELOPE_STATE r_CurrentEnvelopeStates [`NUM_VOICE_OPERATORS];
logic `ENVELOPE_LEVEL r_CurrentEnvelopeLevels [`NUM_VOICE_OPERATORS];

// function automatic `ENVELOPE_REGION getEnvelopeStateRegion;
//     input `ENVELOPE_STATE state;
//     getEnvelopeStateRegion = state[16:14];
// endfunction

// function automatic `ENVELOPE_LEVEL getEnvelopeStateLevel;
//     input `ENVELOPE_STATE state;
//     getEnvelopeStateLevel = state[13:0];
// endfunction





`define LEVEL_MAX  14'h3fff

// function EnvelopeLevel_t increaseLevel(EnvelopeLevel_t level, EnvelopeRate_t rate);
//     if (level > `LEVEL_MAX - extendRateConfig(rate))
//         return `LEVEL_MAX;
//     else
//         return level + extendRateConfig(rate);
// endfunction

function automatic `ENVELOPE_LEVEL increaseLevel;
    input `ENVELOPE_LEVEL level;
    input `ENVELOPE_RATE rate;
    if (level > `LEVEL_MAX - extendRateConfig(rate))
        increaseLevel = `LEVEL_MAX;
    else
        increaseLevel = level + extendRateConfig(rate);
endfunction

// function EnvelopeLevel_t decreaseLevel(EnvelopeLevel_t level, EnvelopeRate_t rate);
//     if (extendRateConfig(rate) > level)
//         return 0;
//     else
//         return level - extendRateConfig(rate);
// endfunction

function automatic `ENVELOPE_LEVEL decreaseLevel;
    input `ENVELOPE_LEVEL level;
    input `ENVELOPE_RATE rate;
    if (extendRateConfig(rate) > level)
        decreaseLevel = 0;
    else
        decreaseLevel = level - extendRateConfig(rate);
endfunction

// function EnvelopeLevel_t extendRateConfig(EnvelopeRate_t rateConfig);
    // return {2'b0, rateConfig};
// endfunction

function automatic `ENVELOPE_LEVEL extendRateConfig;
    input `ENVELOPE_RATE rate;
    extendRateConfig = {2'b0, rate};
endfunction


logic `VOICE_OPERATOR_ID r_VoiceOperator [4];
logic `ALGORITHM_WORD r_AlgorithmWord [4];


logic r_DoEnvelopCalc;
logic `ENVELOPE_STATE r_EnvelopeState;
logic `ENVELOPE_LEVEL r_EnvelopeLevel;
logic r_NoteOn;

logic `ENVELOPE_LEVEL r_AttackLevel;
logic `ENVELOPE_LEVEL r_SustainLevel;
logic `ENVELOPE_RATE r_AttackRate;
logic `ENVELOPE_RATE r_DecayRate;
logic `ENVELOPE_RATE r_ReleaseRate;

logic signed [15:0] r_RawWaveform;
logic signed [31:0] r_AttenuatedWaveformProduct [3];


always_ff @ (posedge i_Clock) begin

    // TODO: Ensure that two of these parameters are shared by each of four block RAMs.
    if (i_EnvelopeConfigWriteEnable[0]) r_AttackLevelConfig[i_ConfigWriteAddr] <= i_ConfigWriteData[13:0];
    if (i_EnvelopeConfigWriteEnable[1]) r_SustainLevelConfig[i_ConfigWriteAddr] <= i_ConfigWriteData[13:0];
    if (i_EnvelopeConfigWriteEnable[2]) r_AttackRateConfig[i_ConfigWriteAddr] <= i_ConfigWriteData[11:0];
    if (i_EnvelopeConfigWriteEnable[3]) r_DecayRateConfig[i_ConfigWriteAddr] <= i_ConfigWriteData[11:0];
    if (i_EnvelopeConfigWriteEnable[4]) r_ReleaseRateConfig[i_ConfigWriteAddr] <= i_ConfigWriteData[11:0];

    // Clock 1
    // ----------------------------------------------------------

    r_DoEnvelopCalc <= w_DoEnvelopeCalc;
    r_EnvelopeState <= r_CurrentEnvelopeStates[i_VoiceOperator];
    r_EnvelopeLevel <= r_CurrentEnvelopeLevels[i_VoiceOperator];
    r_NoteOn <= i_NoteOn;
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

    case (r_EnvelopeState)

        // `MUTE:
        default: begin
            r_CurrentEnvelopeLevels[r_VoiceOperator[0]] <= 0;
            if (r_NoteOn) begin
                r_CurrentEnvelopeStates[r_VoiceOperator[0]] <= `ATTACK;
            end
        end

        `ATTACK: begin
            if (r_DoEnvelopCalc) begin
                r_CurrentEnvelopeLevels[r_VoiceOperator[0]] <= increaseLevel(r_EnvelopeLevel, r_AttackRate);
            end

            if ( ! r_NoteOn) begin
                r_CurrentEnvelopeStates[r_VoiceOperator[0]] <= `RELEASE;
            end
            else if (r_EnvelopeLevel >= r_AttackLevel) begin
                r_CurrentEnvelopeStates[r_VoiceOperator[0]] <= `DECAY;
            end
        end

        `DECAY: begin
            if (r_DoEnvelopCalc) begin
                r_CurrentEnvelopeLevels[r_VoiceOperator[0]] <= decreaseLevel(r_EnvelopeLevel, r_DecayRate);
            end

            if ( ! r_NoteOn) begin
                r_CurrentEnvelopeStates[r_VoiceOperator[0]] <= `RELEASE;
            end
            else if (r_EnvelopeLevel <= r_SustainLevel) begin
                r_CurrentEnvelopeStates[r_VoiceOperator[0]] <= `SUSTAIN;
            end
        end

        `SUSTAIN: begin
            r_CurrentEnvelopeLevels[r_VoiceOperator[0]] <= r_SustainLevel;
            if ( ! r_NoteOn) begin
                r_CurrentEnvelopeStates[r_VoiceOperator[0]] <= `RELEASE;
            end
        end

        `RELEASE: begin
            if (r_DoEnvelopCalc) begin
                r_CurrentEnvelopeLevels[r_VoiceOperator[0]] <= decreaseLevel(r_EnvelopeLevel, r_ReleaseRate);
            end
            if (r_EnvelopeLevel == 0) begin
                r_CurrentEnvelopeStates[r_VoiceOperator[0]] <= `MUTE;
            end
        end

    endcase

    // ----------------------------------------------------------

    // Clocks 2-5 : Attenuate output sample
    // ----------------------------------------------------------

    r_AttenuatedWaveformProduct[0] <= r_RawWaveform * $signed({1'b0, r_EnvelopeLevel, 1'b0});
    r_AttenuatedWaveformProduct[1] <= r_AttenuatedWaveformProduct[0];
    r_AttenuatedWaveformProduct[2] <= r_AttenuatedWaveformProduct[1];
    o_Waveform <= {r_AttenuatedWaveformProduct[2][30:16], 1'b0};

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
