
// `include "registers.svh"
`include "voice.svh"

module synth (
    input i_Clock,
    input i_Reset,


    // TODO: Change this interface into an SPI-like one.
    // Possibly actually SPI.
    // TODO: Allow reading registers as well.
    input logic [7:0] i_RegisterNumber,
    input logic [23:0] i_RegisterValue,
    input logic i_RegisterWriteEnable,


    // TODO: Use PWM instead of outputting the full sample
    // output [31:0] o_Sample
    output [23:0] o_Sample

);


VoiceRegisters_t [2:1] r_VoiceRegisters;

always_ff @ (posedge i_Clock) begin
    if (i_Reset) begin
        // TODO
        r_VoiceRegisters[1].KeyOn <= 0;
        r_VoiceRegisters[2].KeyOn <= 0;
    end
    else if (i_RegisterWriteEnable) begin
        case (i_RegisterNumber)
            // TODO: `define or enum these register names
            // TODO: Optimize lookup tables?
            // TODO: Delegate this to the Voice module itself? The modules
            //       can be parameterized with a prefix ID bit sequence.

            // Some of these might make sense to set all at once though.
            // For example, with a 32-bit value we can set the key-on value
            // of all voices atomically. That might not be necessary though.

            8'h00: r_VoiceRegisters[1].Algorithm <= i_RegisterValue[0];
            8'h01: r_VoiceRegisters[1].Operator[1].AmplitudeFactor <= i_RegisterValue[15:0];
            8'h02: r_VoiceRegisters[1].Operator[1].Frequency <= i_RegisterValue;
            8'h03: r_VoiceRegisters[1].Operator[2].AmplitudeFactor <= i_RegisterValue[15:0];
            8'h04: r_VoiceRegisters[1].Operator[2].Frequency <= i_RegisterValue;
            8'h05: r_VoiceRegisters[1].KeyOn <= i_RegisterValue[0];

            8'h10: r_VoiceRegisters[2].Algorithm <= i_RegisterValue[0];
            8'h11: r_VoiceRegisters[2].Operator[1].AmplitudeFactor <= i_RegisterValue[15:0];
            8'h12: r_VoiceRegisters[2].Operator[1].Frequency <= i_RegisterValue;
            8'h13: r_VoiceRegisters[2].Operator[2].AmplitudeFactor <= i_RegisterValue[15:0];
            8'h14: r_VoiceRegisters[2].Operator[2].Frequency <= i_RegisterValue;
            8'h15: r_VoiceRegisters[2].KeyOn <= i_RegisterValue[0];

            default: /* do nothing */;
        endcase
    end
end





// TODO: Make in a loop?
voice voice1 (
    .i_Clock (i_Clock),
    .i_Reset (i_Reset),
    .r_Registers(r_VoiceRegisters[1]),
    .o_Sample(w_Voice1Sample)
);

voice voice2 (
    .i_Clock (i_Clock),
    .i_Reset (i_Reset),
    .r_Registers(r_VoiceRegisters[2]),
    .o_Sample(w_Voice2Sample)
);

logic signed [23:0] w_Voice1Sample;
logic signed [23:0] w_Voice2Sample;


// TODO: What's the best way to average the outputs?
// Should each voice be given an amplitude factor?
// Or do we rely on the controlling software to ensure that all
// active operator amplitudes add up to less than 100%?
// I guess if someone wants to let the values overflow or clip
// then that's something they might want to do.
assign o_Sample =
      w_Voice1Sample / 2
    + w_Voice2Sample / 2;






endmodule
