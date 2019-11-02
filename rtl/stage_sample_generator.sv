
// Collect carrier operators into subsamples, then complete samples

`include "synth.svh"

module stage_sample_generator (
    input i_Clock,

    input VoiceOperatorID_t i_VoiceOperator,

    // verilator lint_off UNUSED
    input AlgorithmWord_t i_AlgorithmWord,
    // verilator lint_on UNUSED

    input logic signed [15:0] i_OperatorOutput,

    output logic o_SampleReady,
    output logic signed [15:0] o_Sample

);


VoiceOperatorID_t r_VoiceOperator [4];
logic r_IsCarrier [4];

logic signed [31:0] r_CompensatedCarrierOutputProduct [3];
logic signed [15:0] r_CompensatedCarrierOutput;

logic signed [15:0] w_CarrierCompsensationFactor;
logic signed [20:0] w_SignExtendedCarrierOutput;
logic signed [20:0] r_SampleBuffer;



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

    w_SignExtendedCarrierOutput = {{5{r_CompensatedCarrierOutput[15]}}, r_CompensatedCarrierOutput};

end


always_ff @ (posedge i_Clock) begin

    // Stage 1: Carrier Compensation
    // (4 clock cycles)
    // -------------------------------------------------------

    r_VoiceOperator[0] <= i_VoiceOperator;
    r_VoiceOperator[1] <= r_VoiceOperator[0];
    r_VoiceOperator[2] <= r_VoiceOperator[1];
    r_VoiceOperator[3] <= r_VoiceOperator[2];

    r_IsCarrier[0] <= getIsCarrier(i_AlgorithmWord);
    r_IsCarrier[1] <= r_IsCarrier[0];
    r_IsCarrier[2] <= r_IsCarrier[1];
    r_IsCarrier[3] <= r_IsCarrier[2];

    r_CompensatedCarrierOutputProduct[0] <= w_CarrierCompsensationFactor * i_OperatorOutput;
    r_CompensatedCarrierOutputProduct[1] <= r_CompensatedCarrierOutputProduct[0];
    r_CompensatedCarrierOutputProduct[2] <= r_CompensatedCarrierOutputProduct[1];
    r_CompensatedCarrierOutput <= {r_CompensatedCarrierOutputProduct[2][30:16], 1'b0};

    // -------------------------------------------------------


    // Stage 2: Carrier output collection
    // -------------------------------------------------------

    // If the previous clock output a sample, then this clock should
    // reset the buffer.
    if (o_SampleReady) begin
        if (r_IsCarrier[3]) begin
            r_SampleBuffer <= w_SignExtendedCarrierOutput;
        end
        else begin
            r_SampleBuffer <= 0;
        end
    end
    else if (r_IsCarrier[3]) begin
        r_SampleBuffer <= r_SampleBuffer + w_SignExtendedCarrierOutput;
    end

    o_SampleReady <= r_VoiceOperator[3] == 8'hff;

    // -------------------------------------------------------

end

assign o_Sample = r_SampleBuffer[20:5];



endmodule
