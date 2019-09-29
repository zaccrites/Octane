
`include "core.svh"


module synth (
    input logic i_Clock,
    input logic i_Reset,

    // TODO: Store in registers for individual operators and voices
    // input logic unsigned [15:0] i_PhaseStep,

    // TODO: SPI-like interface
    input logic unsigned [15:0] i_RegisterNumber,
    // verilator lint_off UNUSED
    input logic unsigned [15:0] i_RegisterValue,
    // verilator lint_on UNUSED
    input logic i_RegisterWriteEnable,

    // TODO: PWM, or output to DAC via SPI?
    output logic o_SampleReady,
    output logic unsigned [15:0] o_Sample
);





// verilator lint_off UNUSED
logic signed [19:0] r_SampleBuffer;
logic signed [15:0] w_Subsample;
logic w_SampleReady;
logic w_SubsampleReady;
core core0 (
    .i_Clock        (i_Clock),
    // .i_Reset        (i_Reset),
    .i_Config        (r_CoreConfig),
    .o_Subsample       (w_Subsample),
    .o_SubsampleReady (w_SubsampleReady),
    .o_SampleReady    (w_SampleReady)
);

assign o_SampleReady = w_SampleReady;

logic signed [19:0] w_SignExtendedSubsample;
assign w_SignExtendedSubsample = {{4{w_Subsample[15]}}, w_Subsample};


CoreConfig_t r_CoreConfig;


logic r_StartingNewCycle;
always_ff @ (posedge i_Clock) begin

    r_StartingNewCycle <= w_SampleReady;
    if (i_Reset || r_StartingNewCycle) begin
        r_SampleBuffer <= w_SignExtendedSubsample;
        // $display("Resetting sample buffer");
    end
    else if (w_SubsampleReady) begin
        r_SampleBuffer <= r_SampleBuffer + w_SignExtendedSubsample;
        // $display("Got subsample!");
    end

    if (w_SampleReady) begin
        o_Sample <= r_SampleBuffer[19:4];
        // $display("Completed sample!");
    end


end



integer voiceNumber;
integer operatorNumber;
`define VOICE_CONFIG     r_CoreConfig.VoiceConfigs[voiceNumber - 1]
`define OPERATOR_CONFIG  `VOICE_CONFIG.OperatorConfigs[operatorNumber - 1]


always_ff @ (posedge i_Clock) begin
    if (i_Reset) begin
        for (voiceNumber = 1; voiceNumber <= 16; voiceNumber = voiceNumber + 1)
            r_CoreConfig.VoiceConfigs[voiceNumber - 1].KeyOn <= 0;
    end


    if (i_RegisterWriteEnable) begin
        for (voiceNumber = 1; voiceNumber <= 16; voiceNumber = voiceNumber + 1) begin
            case (i_RegisterNumber)
                {voiceNumber[4:0], 3'b000, 8'h00}: `VOICE_CONFIG.KeyOn <= i_RegisterValue[0];

                default: /* do nothing */;
            endcase

            for (operatorNumber = 1; operatorNumber <= 6; operatorNumber = operatorNumber + 1) begin
                case (i_RegisterNumber)
                    {voiceNumber[4:0], operatorNumber[2:0], 8'h00}: `OPERATOR_CONFIG.PhaseStep <= i_RegisterValue;
                    {voiceNumber[4:0], operatorNumber[2:0], 8'h01}: `OPERATOR_CONFIG.Waveform  <= i_RegisterValue[0];
                    {voiceNumber[4:0], operatorNumber[2:0], 8'h02}: `OPERATOR_CONFIG.EnvelopeLevel  <= i_RegisterValue;


                    default: /* do nothing */;
                endcase
            end

        end
            // 16'h0100: r_CoreConfig.VoiceConfigs[0].OperatorConfigs[0].PhaseStep <= i_RegisterValue[];
    end


end


endmodule
