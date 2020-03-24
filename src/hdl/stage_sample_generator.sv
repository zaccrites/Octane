
// Collect carrier operators into subsamples, then complete samples

`include "synth.svh"

module stage_sample_generator (
    input i_Clock,

    input `VOICE_OPERATOR_ID i_VoiceOperator,

    // verilator lint_off UNUSED
    input `ALGORITHM_WORD i_AlgorithmWord,
    // verilator lint_on UNUSED

    input logic signed [15:0] i_OperatorOutput,

    output logic o_SampleReady,
    output logic signed [15:0] o_Sample

);


logic `VOICE_OPERATOR_ID r_VoiceOperator [16];
logic r_IsCarrier [16];

logic signed [32:0] r_CompensatedCarrierOutputProduct [16];
logic signed [15:0] w_CompensatedCarrierOutput;

logic signed [15:0] r_CarrierCompsensationFactor [16];
logic signed [15:0] w_CarrierCompsensationFactor;

logic signed [20:0] w_SignExtendedCarrierOutput;
logic signed [20:0] r_SampleBuffer;

logic signed [15:0] r_OperatorOutput [16];


always_comb begin

    // Carrier compensation table
    case (getNumCarriers(i_AlgorithmWord))
        3'd0: w_CarrierCompsensationFactor = 16'h7fff;
        3'd1: w_CarrierCompsensationFactor = 16'h4000;
        3'd2: w_CarrierCompsensationFactor = 16'h2aaa;
        3'd3: w_CarrierCompsensationFactor = 16'h2000;
        3'd4: w_CarrierCompsensationFactor = 16'h1999;
        3'd5: w_CarrierCompsensationFactor = 16'h1555;
        3'd6: w_CarrierCompsensationFactor = 16'h1249;
        3'd7: w_CarrierCompsensationFactor = 16'h1000;
    endcase

    w_CompensatedCarrierOutput = {r_CompensatedCarrierOutputProduct[15][30:16], 1'b0};
    w_SignExtendedCarrierOutput = {{5{w_CompensatedCarrierOutput[15]}}, w_CompensatedCarrierOutput};

end


integer i;
always_ff @ (posedge i_Clock) begin

    // Stage 1: Carrier Compensation
    // (16 clock cycles)
    // -------------------------------------------------------

    r_VoiceOperator[0] <= i_VoiceOperator;
    r_IsCarrier[0] <= getIsCarrier(i_AlgorithmWord);
    r_CarrierCompsensationFactor[0] <= w_CarrierCompsensationFactor;
    r_OperatorOutput[0] <= i_OperatorOutput;

    for (i = 1; i <= 15; i++) begin
        r_VoiceOperator[i] <= r_VoiceOperator[i - 1];
        r_IsCarrier[i] <= r_IsCarrier[i - 1];
        r_CarrierCompsensationFactor[i] <= r_CarrierCompsensationFactor[i - 1];
        r_OperatorOutput[i] <= r_OperatorOutput[i - 1];
    end

    // Pipeline a (16 bit unsigned)x(16 bit signed) multiply by repeated addition
    r_CompensatedCarrierOutputProduct[0] <= w_CarrierCompsensationFactor[0] ? {{17{i_OperatorOutput[15]}}, i_OperatorOutput} : 0;
    r_CompensatedCarrierOutputProduct[1] <= r_CompensatedCarrierOutputProduct[0] + (r_CarrierCompsensationFactor[0][1] ? {{16{r_OperatorOutput[0][15]}}, r_OperatorOutput[0], 1'b0} : 0);
    r_CompensatedCarrierOutputProduct[2] <= r_CompensatedCarrierOutputProduct[1] + (r_CarrierCompsensationFactor[1][2] ? {{15{r_OperatorOutput[1][15]}}, r_OperatorOutput[1], 2'b0} : 0);
    r_CompensatedCarrierOutputProduct[3] <= r_CompensatedCarrierOutputProduct[2] + (r_CarrierCompsensationFactor[2][3] ? {{14{r_OperatorOutput[2][15]}}, r_OperatorOutput[2], 3'b0} : 0);
    r_CompensatedCarrierOutputProduct[4] <= r_CompensatedCarrierOutputProduct[3] + (r_CarrierCompsensationFactor[3][4] ? {{13{r_OperatorOutput[3][15]}}, r_OperatorOutput[3], 4'b0} : 0);
    r_CompensatedCarrierOutputProduct[5] <= r_CompensatedCarrierOutputProduct[4] + (r_CarrierCompsensationFactor[4][5] ? {{12{r_OperatorOutput[4][15]}}, r_OperatorOutput[4], 5'b0} : 0);
    r_CompensatedCarrierOutputProduct[6] <= r_CompensatedCarrierOutputProduct[5] + (r_CarrierCompsensationFactor[5][6] ? {{11{r_OperatorOutput[5][15]}}, r_OperatorOutput[5], 6'b0} : 0);
    r_CompensatedCarrierOutputProduct[7] <= r_CompensatedCarrierOutputProduct[6] + (r_CarrierCompsensationFactor[6][7] ? {{10{r_OperatorOutput[6][15]}}, r_OperatorOutput[6], 7'b0} : 0);
    r_CompensatedCarrierOutputProduct[8] <= r_CompensatedCarrierOutputProduct[7] + (r_CarrierCompsensationFactor[7][8] ? {{9{r_OperatorOutput[7][15]}}, r_OperatorOutput[7], 8'b0} : 0);
    r_CompensatedCarrierOutputProduct[9] <= r_CompensatedCarrierOutputProduct[8] + (r_CarrierCompsensationFactor[8][9] ? {{8{r_OperatorOutput[8][15]}}, r_OperatorOutput[8], 9'b0} : 0);
    r_CompensatedCarrierOutputProduct[10] <= r_CompensatedCarrierOutputProduct[9] + (r_CarrierCompsensationFactor[9][10] ? {{7{r_OperatorOutput[9][15]}}, r_OperatorOutput[9], 10'b0} : 0);
    r_CompensatedCarrierOutputProduct[11] <= r_CompensatedCarrierOutputProduct[10] + (r_CarrierCompsensationFactor[10][11] ? {{6{r_OperatorOutput[10][15]}}, r_OperatorOutput[10], 11'b0} : 0);
    r_CompensatedCarrierOutputProduct[12] <= r_CompensatedCarrierOutputProduct[11] + (r_CarrierCompsensationFactor[11][12] ? {{5{r_OperatorOutput[11][15]}}, r_OperatorOutput[11], 12'b0} : 0);
    r_CompensatedCarrierOutputProduct[13] <= r_CompensatedCarrierOutputProduct[12] + (r_CarrierCompsensationFactor[12][13] ? {{4{r_OperatorOutput[12][15]}}, r_OperatorOutput[12], 13'b0} : 0);
    r_CompensatedCarrierOutputProduct[14] <= r_CompensatedCarrierOutputProduct[13] + (r_CarrierCompsensationFactor[13][14] ? {{3{r_OperatorOutput[13][15]}}, r_OperatorOutput[13], 14'b0} : 0);
    r_CompensatedCarrierOutputProduct[15] <= r_CompensatedCarrierOutputProduct[14] + (r_CarrierCompsensationFactor[14][15] ? {{2{r_OperatorOutput[14][15]}}, r_OperatorOutput[14], 15'b0} : 0);

    // -------------------------------------------------------


    // Stage 2: Carrier output collection
    // -------------------------------------------------------

    // If the previous clock output a sample, then this clock should
    // reset the buffer.
    if (o_SampleReady) begin
        if (r_IsCarrier[15]) begin
            r_SampleBuffer <= w_SignExtendedCarrierOutput;
        end
        else begin
            r_SampleBuffer <= 0;
        end
    end
    else if (r_IsCarrier[15]) begin
        r_SampleBuffer <= r_SampleBuffer + w_SignExtendedCarrierOutput;
    end

    o_SampleReady <= r_VoiceOperator[15] == `NUM_VOICE_OPERATORS - 1;
    // $display("ID = %d", r_VoiceOperator[15]);

    // -------------------------------------------------------

end

assign o_Sample = r_SampleBuffer[20:5];



endmodule
