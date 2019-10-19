
module synth (
    input logic i_Clock,
    input logic i_Reset,

    // TODO: Use SPI interface for input and output
    input logic i_RegisterWriteEnable,
    input logic [15:0] i_RegisterNumber,
    input logic [7:0] i_RegisterValue,
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




/// Voice configuration registers
logic r_NoteOn [32];
logic [7:0] r_Algorithm [32];

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
logic [7:0] w_VoiceOpRegIndex;
assign w_VoiceOpRegIndex = i_RegisterNumber[7:0];
logic [4:0] w_VoiceRegIndex;
assign w_VoiceRegIndex = i_RegisterNumber[4:0];
//
always_ff @ (posedge i_Clock) begin

    if (i_RegisterWriteEnable) begin
        case (i_RegisterNumber[15:8])
            {2'b11, 6'h00}: r_PhaseStep[w_VoiceOpRegIndex][15:8] <= i_RegisterValue;
            {2'b11, 6'h01}: r_PhaseStep[w_VoiceOpRegIndex][7:0] <= i_RegisterValue;
            {2'b11, 6'h02}: r_Waveform[w_VoiceOpRegIndex][15:8] <= i_RegisterValue;
            {2'b11, 6'h03}: r_Waveform[w_VoiceOpRegIndex][7:0] <= i_RegisterValue;
            //
            {2'b11, 6'h04}: r_EnvelopeL1[w_VoiceOpRegIndex] <= i_RegisterValue;
            {2'b11, 6'h05}: r_EnvelopeL2[w_VoiceOpRegIndex] <= i_RegisterValue;
            {2'b11, 6'h06}: r_EnvelopeL3[w_VoiceOpRegIndex] <= i_RegisterValue;
            {2'b11, 6'h07}: r_EnvelopeL4[w_VoiceOpRegIndex] <= i_RegisterValue;
            {2'b11, 6'h08}: r_EnvelopeR1[w_VoiceOpRegIndex] <= i_RegisterValue;
            {2'b11, 6'h09}: r_EnvelopeR2[w_VoiceOpRegIndex] <= i_RegisterValue;
            {2'b11, 6'h0a}: r_EnvelopeR3[w_VoiceOpRegIndex] <= i_RegisterValue;
            {2'b11, 6'h0b}: r_EnvelopeR4[w_VoiceOpRegIndex] <= i_RegisterValue;
            //
            //
            {2'b10, 6'h00}: r_NoteOn[w_VoiceRegIndex] <= i_RegisterValue[0];
            {2'b10, 6'h01}: r_Algorithm[w_VoiceRegIndex] <= i_RegisterValue;
            {2'b10, 6'h02}: r_CarrierComp[w_VoiceRegIndex][15:8] <= i_RegisterValue;  // TODO: Remove
            {2'b10, 6'h03}: r_CarrierComp[w_VoiceRegIndex][7:0] <= i_RegisterValue;  // TODO: Remove

            default: /* ignore writes to invalid registers */ ;
        endcase
    end
end



/// Track the currently active voice operator
logic unsigned [7:0] r_CycleNumber [31:0];
`define CYCLE_VOICE(index)  r_CycleNumber[index][4:0]
`define CYCLE_OPERATOR(index)  r_CycleNumber[index][7:5]

// Cycle counter state machine
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
        r_CycleNumber[0] <= 0;
    else
        r_CycleNumber[0] <= r_CycleNumber[0] + 1;
end






typedef enum logic [2:0] {
    MUTE,
    ATTACK,
    DECAY,
    RECOVER,
    SUSTAIN,
    RELEASE
} EnvelopeState_t;


// TODO: Can these be stored together in a single block RAM?
EnvelopeState_t r_EnvelopeState [256];
// logic signed [12:0] r_EnvelopeLevel [256];
logic unsigned [7:0] r_EnvelopeLevel [256];

logic unsigned [7:0] r_EnvelopeClockDivider;

// Envelope generation state machine
always_ff @ (posedge i_Clock) begin
    `define STATE r_EnvelopeState[r_CycleNumber[0]]
    `define L   r_EnvelopeLevel[r_CycleNumber[0]]
    `define L1  r_EnvelopeL1[r_CycleNumber[0]]
    `define L2  r_EnvelopeL2[r_CycleNumber[0]]
    `define L3  r_EnvelopeL3[r_CycleNumber[0]]
    `define L4  r_EnvelopeL4[r_CycleNumber[0]]
    `define R1  r_EnvelopeR1[r_CycleNumber[0]]
    `define R2  r_EnvelopeR2[r_CycleNumber[0]]
    `define R3  r_EnvelopeR3[r_CycleNumber[0]]
    `define R4  r_EnvelopeR4[r_CycleNumber[0]]
    `define NOTE_ON  r_NoteOn[`CYCLE_VOICE(0)]
    `define NOTE_OFF  ( ! `NOTE_ON)


    if (r_CycleNumber[0] == 0) begin
        if (r_EnvelopeClockDivider[6])
            r_EnvelopeClockDivider <= 0;
        else
            r_EnvelopeClockDivider <= r_EnvelopeClockDivider + 1;
    end

    if (r_EnvelopeClockDivider[6]) begin
        case (`STATE)
            /* MUTE */ default: begin
                `L <= 0;
                if (`NOTE_ON) `STATE <= ATTACK;
            end

            ATTACK: begin
                `L <= `L + `R1;
                if (`NOTE_OFF)       `STATE <= RELEASE;
                else if (`L >= `L1)  `STATE <= DECAY;
            end

            DECAY: begin
                `L <= `L - `R2;
                if (`NOTE_OFF)       `STATE <= RELEASE;
                else if (`L <= `L2)  `STATE <= RECOVER;
            end

            RECOVER: begin
                `L <= `L + `R3;
                if (`NOTE_OFF)       `STATE <= RELEASE;
                else if (`L >= `L3)  `STATE <= SUSTAIN;
            end

            SUSTAIN: begin
                `L <= `L3;
                if (`NOTE_OFF) `STATE <= RELEASE;
            end

            RELEASE: begin
                `L <= `L - `R4;
                if (`L <= `L4) `STATE <= MUTE;
            end
        endcase
    end


    `undef L
    `undef L1
    `undef L2
    `undef L3
    `undef L4
    `undef R1
    `undef R2
    `undef R3
    `undef R4
    `undef NOTE_ON
end





logic signed [15:0] w_RawWaveformValue;
waveform_generator wavegen (
    .i_Clock    (i_Clock),
    .i_Phase    (r_Phase[15:3]),
    .i_Waveform (0),  // TODO
    .o_Value    (w_RawWaveformValue)
);


/// Voice operator phase accumulator
logic unsigned [15:0] r_PhaseAcc [256];

/// Modulated voice operator phase
logic unsigned [15:0] r_Phase;




logic signed [15:0] r_WaveformValue [31:4];



logic signed [15:0] r_Subsample;
logic r_SubsampleReady;






// TODO: Reduce range on this and other pipeline registers (use Verilator warnings about UNUSED to trim them down)
logic signed [31:0] r_WaveformAmplitudeProduct [31:0];

logic signed [31:0] r_EnvelopeFactorProduct [2:0];
logic signed [15:0] r_EnvelopeFactor;



// Subsample generation
integer i;
always_ff @ (posedge i_Clock) begin

    // Stage 1: accumulate and modulate phase
    // (1 clock cycle)
    if (r_NoteOn[`CYCLE_VOICE(0)])
        r_PhaseAcc[r_CycleNumber[0]] <= r_PhaseAcc[r_CycleNumber[0]] + r_PhaseStep[r_CycleNumber[0]];
    else
        r_PhaseAcc[r_CycleNumber[0]] <= 0;
    r_Phase <= r_PhaseAcc[r_CycleNumber[0]];  // TODO: Modulation with feedback

    // Stage 1 (branch 2): Multi-carrier Amplitude Compensation
    // (4 clock cycles)
    r_EnvelopeFactorProduct[0] <= $signed({1'b0, r_EnvelopeLevel[r_CycleNumber[0]], 7'b0}) * r_CarrierComp[`CYCLE_VOICE(0)];
    r_EnvelopeFactorProduct[1] <= r_EnvelopeFactorProduct[0];
    r_EnvelopeFactorProduct[2] <= r_EnvelopeFactorProduct[1];
    r_EnvelopeFactor <= {r_EnvelopeFactorProduct[2][30:16], 1'b0};

    // Stage 2: waveform generation
    // (see waveform_generator module)
    // (3 clock cycles)
    r_CycleNumber[1] <= r_CycleNumber[0];
    r_CycleNumber[2] <= r_CycleNumber[1];
    r_CycleNumber[3] <= r_CycleNumber[2];


    // Stage 3: Waveform Amplitude
    //
    // The waveform amplitude can be attenuated by the current position
    // in the envelope, but is also attenuated by the number of carriers
    // in the currently selected algorithm.
    //
    // The DX7 used a separate multiply stage to attenuate for carriers
    // before applying the envelope attenuation, but we will do it in
    // software by combining the current envelope level and the carrier
    // attenuation factor into a single multiplicand.
    //
    // (4 clock cycles)
    r_WaveformAmplitudeProduct[4] <= w_RawWaveformValue * r_EnvelopeFactor;
    r_WaveformAmplitudeProduct[5] <= r_WaveformAmplitudeProduct[4];
    r_WaveformAmplitudeProduct[6] <= r_WaveformAmplitudeProduct[5];
    r_WaveformValue[7] <= {r_WaveformAmplitudeProduct[6][30:16], 1'b0};
    //
    r_CycleNumber[4] <= r_CycleNumber[3];
    r_CycleNumber[5] <= r_CycleNumber[4];
    r_CycleNumber[6] <= r_CycleNumber[5];
    r_CycleNumber[7] <= r_CycleNumber[6];


    // Stage 4: filtering
    // TODO


    // Stage 5: stall until we reach 32 cycles
    for (i = 8; i < 32; i = i + 1) begin
        r_WaveformValue[i] <= r_WaveformValue[i - 1];
        r_CycleNumber[i] <= r_CycleNumber[i - 1];
    end

    r_Subsample <= r_WaveformValue[31];
    r_SubsampleReady <= `CYCLE_OPERATOR(31) == 3'd7;
    o_SampleReady <= r_CycleNumber[31] == 8'd255;
end



logic signed [21:0] r_SampleBuffer;
logic signed [21:0] w_SignExtendedSubsample;
assign w_SignExtendedSubsample = {{6{r_Subsample[15]}}, r_Subsample};

// Subsample combination and output
always_ff @ (posedge i_Clock) begin

    if (i_Reset || o_SampleReady)
        r_SampleBuffer <= w_SignExtendedSubsample;
    else if (r_SubsampleReady)
        r_SampleBuffer <= r_SampleBuffer + w_SignExtendedSubsample;

    if (o_SampleReady)
        o_Sample <= r_SampleBuffer[20:5];

end




endmodule
