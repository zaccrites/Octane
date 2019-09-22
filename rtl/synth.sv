
// `include "registers.svh"
`include "voice.svh"

module synth
#(
    localparam NUM_VOICES = 8
)
(

    input i_Clock,
    input i_Reset,


    // TODO: Change this interface into an SPI-like one.
    // Possibly actually SPI.
    // TODO: Allow reading registers as well.
    input logic [11:0] i_RegisterNumber,
    input logic [23:0] i_RegisterValue,
    input logic i_RegisterWriteEnable,


    // TODO: Use PWM instead of outputting the full sample
    // output [31:0] o_Sample
    output [23:0] o_Sample

);


logic [5:0] voiceID;
VoiceRegisters_t [NUM_VOICES:1] r_VoiceRegisters;

always_ff @ (posedge i_Clock) begin
    if (i_Reset) begin
        for (voiceID = 1; voiceID <= NUM_VOICES; voiceID = voiceID + 1)
            r_VoiceRegisters[voiceID].KeyOn <= 0;
    end
    else if (i_RegisterWriteEnable) begin

        for (voiceID = 1; voiceID <= NUM_VOICES; voiceID = voiceID + 1) begin
            case (i_RegisterNumber)
                // TODO: `define or enum these register names
                // TODO: Optimize lookup tables?
                // TODO: Delegate this to the Voice module itself? The modules
                //       can be parameterized with a prefix ID bit sequence.

                // Some of these might make sense to set all at once though.
                // For example, with a 32-bit value we can set the key-on value
                // of all voices atomically. That might not be necessary though.

                {voiceID, 6'h00}: r_VoiceRegisters[voiceID].Algorithm <= i_RegisterValue[0];
                {voiceID, 6'h01}: r_VoiceRegisters[voiceID].Operator[1].AmplitudeFactor <= i_RegisterValue[15:0];
                {voiceID, 6'h02}: r_VoiceRegisters[voiceID].Operator[1].Frequency <= i_RegisterValue;
                {voiceID, 6'h03}: r_VoiceRegisters[voiceID].Operator[2].AmplitudeFactor <= i_RegisterValue[15:0];
                {voiceID, 6'h04}: r_VoiceRegisters[voiceID].Operator[2].Frequency <= i_RegisterValue;
                {voiceID, 6'h05}: r_VoiceRegisters[voiceID].KeyOn <= i_RegisterValue[0];

                default: /* do nothing */;
            endcase
        end

    end
end



logic signed [23:0] w_VoiceSamples [NUM_VOICES:1];
genvar i;
for (i = 1; i <= NUM_VOICES; i = i + 1)
    voice voice_instance (
        .i_Clock (i_Clock),
        .i_Reset (i_Reset),
        .r_Registers(r_VoiceRegisters[i]),
        .o_Sample(w_VoiceSamples[i])
    );


// TODO: What's the best way to average the outputs?
// Should each voice be given an amplitude factor?
// Or do we rely on the controlling software to ensure that all
// active operator amplitudes add up to less than 100%?
// I guess if someone wants to let the values overflow or clip
// then that's something they might want to do.
// always_comb begin
//     o_Sample = 0;
//     for (voiceID = 1; voiceID <= NUM_VOICES; voiceID = voiceID + 1)
//         o_Sample = o_Sample + (w_VoiceSamples[voiceID] / NUM_VOICES);
// end

assign o_Sample =
    w_VoiceSamples[1] / 8 +
    w_VoiceSamples[2] / 8 +
    w_VoiceSamples[3] / 8 +
    w_VoiceSamples[4] / 8 +
    w_VoiceSamples[5] / 8 +
    w_VoiceSamples[6] / 8 +
    w_VoiceSamples[7] / 8 +
    w_VoiceSamples[8] / 8;




endmodule
