
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



`define LEVEL_MAX  14'h3fff

function automatic `ENVELOPE_LEVEL increaseLevel;
    input `ENVELOPE_LEVEL level;
    input `ENVELOPE_RATE rate;
    if (level > `LEVEL_MAX - extendRateConfig(rate))
        increaseLevel = `LEVEL_MAX;
    else
        increaseLevel = level + extendRateConfig(rate);
endfunction

function automatic `ENVELOPE_LEVEL decreaseLevel;
    input `ENVELOPE_LEVEL level;
    input `ENVELOPE_RATE rate;
    if (extendRateConfig(rate) > level)
        decreaseLevel = 0;
    else
        decreaseLevel = level - extendRateConfig(rate);
endfunction

function automatic `ENVELOPE_LEVEL extendRateConfig;
    input `ENVELOPE_RATE rate;
    extendRateConfig = {2'b0, rate};
endfunction


logic `VOICE_OPERATOR_ID r_VoiceOperator [15];
logic `ALGORITHM_WORD r_AlgorithmWord [15];


logic r_DoEnvelopCalc;
logic `ENVELOPE_STATE r_EnvelopeState;
logic `ENVELOPE_LEVEL r_EnvelopeLevel [15];
logic r_NoteOn;

logic `ENVELOPE_LEVEL r_AttackLevel;
logic `ENVELOPE_LEVEL r_SustainLevel;
logic `ENVELOPE_RATE r_AttackRate;
logic `ENVELOPE_RATE r_DecayRate;
logic `ENVELOPE_RATE r_ReleaseRate;

logic signed [15:0] r_RawWaveform [15];
logic signed [30:0] r_AttenuatedWaveformProduct [14:1];


integer i;
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
    r_EnvelopeLevel[0] <= r_CurrentEnvelopeLevels[i_VoiceOperator];
    r_NoteOn <= i_NoteOn;
    r_RawWaveform[0] <= i_Waveform;

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
                r_CurrentEnvelopeLevels[r_VoiceOperator[0]] <= increaseLevel(r_EnvelopeLevel[0], r_AttackRate);
            end

            if ( ! r_NoteOn) begin
                r_CurrentEnvelopeStates[r_VoiceOperator[0]] <= `RELEASE;
            end
            else if (r_EnvelopeLevel[0] >= r_AttackLevel) begin
                r_CurrentEnvelopeStates[r_VoiceOperator[0]] <= `DECAY;
            end
        end

        `DECAY: begin
            if (r_DoEnvelopCalc) begin
                r_CurrentEnvelopeLevels[r_VoiceOperator[0]] <= decreaseLevel(r_EnvelopeLevel[0], r_DecayRate);
            end

            if ( ! r_NoteOn) begin
                r_CurrentEnvelopeStates[r_VoiceOperator[0]] <= `RELEASE;
            end
            else if (r_EnvelopeLevel[0] <= r_SustainLevel) begin
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
                r_CurrentEnvelopeLevels[r_VoiceOperator[0]] <= decreaseLevel(r_EnvelopeLevel[0], r_ReleaseRate);
            end
            if (r_EnvelopeLevel[0] == 0) begin
                r_CurrentEnvelopeStates[r_VoiceOperator[0]] <= `MUTE;
            end
        end

    endcase

    // ----------------------------------------------------------

    // Clocks 2-15 : Attenuate output sample
    // ----------------------------------------------------------

    // Pipeline a (14 bit unsigned)x(16 bit signed) multiply by repeated addition
    r_AttenuatedWaveformProduct[1] <= r_EnvelopeLevel[0][0] ? {{15{r_RawWaveform[0][15]}}, r_RawWaveform[0]} : 0;
    r_AttenuatedWaveformProduct[2] <= r_AttenuatedWaveformProduct[1] + (r_EnvelopeLevel[1][1] ? {{14{r_RawWaveform[1][15]}}, r_RawWaveform[1], 1'b0} : 0);
    r_AttenuatedWaveformProduct[3] <= r_AttenuatedWaveformProduct[2] + (r_EnvelopeLevel[2][2] ? {{13{r_RawWaveform[2][15]}}, r_RawWaveform[2], 2'b0} : 0);
    r_AttenuatedWaveformProduct[4] <= r_AttenuatedWaveformProduct[3] + (r_EnvelopeLevel[3][3] ? {{12{r_RawWaveform[3][15]}}, r_RawWaveform[3], 3'b0} : 0);
    r_AttenuatedWaveformProduct[5] <= r_AttenuatedWaveformProduct[4] + (r_EnvelopeLevel[4][4] ? {{11{r_RawWaveform[4][15]}}, r_RawWaveform[4], 4'b0} : 0);
    r_AttenuatedWaveformProduct[6] <= r_AttenuatedWaveformProduct[5] + (r_EnvelopeLevel[5][5] ? {{10{r_RawWaveform[5][15]}}, r_RawWaveform[5], 5'b0} : 0);
    r_AttenuatedWaveformProduct[7] <= r_AttenuatedWaveformProduct[6] + (r_EnvelopeLevel[6][6] ? {{9{r_RawWaveform[6][15]}}, r_RawWaveform[6], 6'b0} : 0);
    r_AttenuatedWaveformProduct[8] <= r_AttenuatedWaveformProduct[7] + (r_EnvelopeLevel[7][7] ? {{8{r_RawWaveform[7][15]}}, r_RawWaveform[7], 7'b0} : 0);
    r_AttenuatedWaveformProduct[9] <= r_AttenuatedWaveformProduct[8] + (r_EnvelopeLevel[8][8] ? {{7{r_RawWaveform[8][15]}}, r_RawWaveform[8], 8'b0} : 0);
    r_AttenuatedWaveformProduct[10] <= r_AttenuatedWaveformProduct[9] + (r_EnvelopeLevel[9][9] ? {{6{r_RawWaveform[9][15]}}, r_RawWaveform[9], 9'b0} : 0);
    r_AttenuatedWaveformProduct[11] <= r_AttenuatedWaveformProduct[10] + (r_EnvelopeLevel[10][10] ? {{5{r_RawWaveform[10][15]}}, r_RawWaveform[10], 10'b0} : 0);
    r_AttenuatedWaveformProduct[12] <= r_AttenuatedWaveformProduct[11] + (r_EnvelopeLevel[11][11] ? {{4{r_RawWaveform[11][15]}}, r_RawWaveform[11], 11'b0} : 0);
    r_AttenuatedWaveformProduct[13] <= r_AttenuatedWaveformProduct[12] + (r_EnvelopeLevel[12][12] ? {{3{r_RawWaveform[12][15]}}, r_RawWaveform[12], 12'b0} : 0);
    r_AttenuatedWaveformProduct[14] <= r_AttenuatedWaveformProduct[13] + (r_EnvelopeLevel[13][13] ? {{2{r_RawWaveform[13][15]}}, r_RawWaveform[13], 13'b0} : 0);

    for (i = 1; i <= 14; i++) begin
        r_RawWaveform[i] <= r_RawWaveform[i - 1];
        r_EnvelopeLevel[i] <= r_EnvelopeLevel[i - 1];
        r_AlgorithmWord[i] <= r_AlgorithmWord[i - 1];
        r_VoiceOperator[i] <= r_VoiceOperator[i - 1];
    end

    // ----------------------------------------------------------

end


assign o_AlgorithmWord = r_AlgorithmWord[14];
assign o_VoiceOperator = r_VoiceOperator[14];
assign o_Waveform = {r_AttenuatedWaveformProduct[14][29:15], 1'b0};


endmodule
