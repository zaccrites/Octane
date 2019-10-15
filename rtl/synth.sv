
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



// Configuration registers

/// Frequency information
logic unsigned [15:0] r_PhaseStep [256];

/// Waveform type and parameters
logic unsigned [15:0] r_Waveform  [256];


/// Voice information
logic r_NoteOn [32];
logic [7:0] r_Algorithm [32];



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
always_ff @ (posedge i_Clock) begin
    if (i_RegisterWriteEnable) begin
        case (i_RegisterNumber[15:8])
            {2'b11, 6'h00}: r_PhaseStep[i_RegisterNumber[7:0]][15:8] <= i_RegisterValue;
            {2'b11, 6'h01}: r_PhaseStep[i_RegisterNumber[7:0]][7:0] <= i_RegisterValue;
            {2'b11, 6'h02}: r_Waveform[i_RegisterNumber[7:0]][15:8] <= i_RegisterValue;
            {2'b11, 6'h03}: r_Waveform[i_RegisterNumber[7:0]][7:0] <= i_RegisterValue;

            {2'b10, 6'h00}: r_NoteOn[i_RegisterNumber[4:0]] <= i_RegisterValue[0];
            {2'b10, 6'h01}: r_Algorithm[i_RegisterNumber[4:0]] <= i_RegisterValue;

            default: /* ignore writes to invalid registers */ ;
        endcase
    end
end



// Envelope generation
always_ff @ (posedge i_Clock) begin

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


/// Track the currently active voice operator
logic unsigned [7:0] r_CycleNumber [32];


logic signed [15:0] r_WaveformValue [27];



logic signed [15:0] r_Subsample;
logic r_SubsampleReady;

// Subsample generation
integer i;
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

    // Stage 1: accumulate and modulate phase
    r_PhaseAcc[r_CycleNumber[0]] <= r_PhaseAcc[r_CycleNumber[0]] + r_PhaseStep[r_CycleNumber[0]];
    r_Phase <= r_PhaseAcc[r_CycleNumber[0]];  // TODO: Modulation with feedback

    // if (r_CycleNumber[0][4:0] == 0) begin
    begin
        // $display("r_PhaseStep[%d] = %d", r_CycleNumber[0], r_PhaseStep[r_CycleNumber[0]]);
        // $display("r_PhaseAcc[%d] = %d", r_CycleNumber[0], r_PhaseAcc[r_CycleNumber[0]]);
    end

    // Stage 2: waveform generation
    //  (see waveform_generator module)
    r_CycleNumber[1] <= r_CycleNumber[0];
    r_CycleNumber[2] <= r_CycleNumber[1];
    r_CycleNumber[3] <= r_CycleNumber[2];

    // Stage 3: waveform amplitude
    // TODO: envelope
    r_CycleNumber[4] <= r_CycleNumber[3];
    r_WaveformValue[0] <= w_RawWaveformValue;

    // Stage 4: filtering
    // TODO

    // Stage 5:
    for (i = 0; i < 32 - 5; i = i + 1) begin
        r_WaveformValue[i + 1] <= r_WaveformValue[i];
        r_CycleNumber[4 + i + 1] <= r_CycleNumber[4 + i];
    end


    if (r_CycleNumber[31] == 8'd255) begin
        // $display("value = %d", r_WaveformValue[25]);
    end



    r_Subsample <= r_WaveformValue[25];

    r_SubsampleReady <= r_CycleNumber[31][7:5] == 3'd7;
    o_SampleReady <= r_CycleNumber[31] == 8'd255;

    if (r_CycleNumber[31][7:5] == 3'd7) begin
        // $display("r_CycleNumber[31] == %d", r_CycleNumber[31]);
    end


end



// Subsample combination and output

logic signed [20:0] r_SampleBuffer;
logic signed [20:0] w_SignExtendedSubsample;
assign w_SignExtendedSubsample = {{5{r_Subsample[15]}}, r_Subsample};

logic r_StartingNewSample;
always_ff @ (posedge i_Clock) begin

    r_StartingNewSample <= o_SampleReady;
    if (i_Reset || r_StartingNewSample) begin
        r_SampleBuffer <= w_SignExtendedSubsample;
    end
    else if (r_SubsampleReady) begin
        r_SampleBuffer <= r_SampleBuffer + w_SignExtendedSubsample;
        // $display("%d", w_SignExtendedSubsample);
    end

    if (o_SampleReady) begin
        o_Sample <= r_SampleBuffer[20:5];
    end

end




endmodule
