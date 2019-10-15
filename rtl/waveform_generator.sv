
module waveform_generator (
    input logic i_Clock,

    input logic unsigned [12:0] i_Phase,

    // 0 - Sine
    // 1 - Square wave (TODO: configurable PWM)
    input logic i_Waveform,

    output logic signed [15:0] o_Value
);

// Each waveform should take three clock cycles!
//  (or at least the same number of clock cycles)


// Sine Wave Generator
//
// The iCE40UP5K has 30 4kb RAM blocks.
// We can use 8 of them to create a 2048-wide 16b sine table.
// Technically we can synthesize a 17th bit from symmetry of the sine function,
// but the DSP elements can only do a 16x16 bit multiply (unless cascaded).
//
// The table gives the first quarter of the waveform, which we
// can index directly using the lower 11 bits of the phase.
// We can use the upper two bits and the sin function's symmetry
// to synthesize the other three quarters.
//
// 00 - Index directly
// 01 - Reverse phase
// 10 - Negate result
// 11 - Reverse phase and negate result
//
// We can then negate the result if the uppermost bit is set
// (the negative half of the period), and reverse the phase
// if the second highest bit is set.
//

// TODO: Can we use the 16th bit of the table here?
// We'd end up with 17 bits, which I'm not sure is useful.
// Alternatively, can we step down to a 14 bit table and
// synthesize a 15th bit to save some block RAM? Do we need to?
initial $readmemh("roms/sine_rom.hex", r_SineTable);
logic unsigned [14:0] r_SineTable[2047:0];
logic unsigned [10:0] r_SineTableIndex;

logic r_NegateSine [2];

/// Quarter-wave sine amplitude
logic unsigned [15:0] r_SineAmplitude;

/// Full-wave sine value (signed amplitude)
logic signed [15:0] w_SineValue;


always_ff @ (posedge i_Clock) begin
    // Stage 1: Compute Index and Negate Flag
    r_NegateSine[0] <= i_Phase[12];
    r_SineTableIndex <= i_Phase[11] ? ~i_Phase[10:0] : i_Phase[10:0];

    // Stage 2: Look up value from table
    r_NegateSine[1] <= r_NegateSine[0];
    r_SineAmplitude <= {1'b0, r_SineTable[r_SineTableIndex]};
end
// Negate the value, if necessary, before output.
assign w_SineValue = r_NegateSine[1] ? ~r_SineAmplitude : r_SineAmplitude;


// Stage 3: Output the computed value
always_ff @ (posedge i_Clock) begin
    case (i_Waveform)
        0 : o_Value <= w_SineValue;
        1 : o_Value <= w_SineValue[15] ? 16'h8000 : 16'h7fff;  // Square wave
    endcase
end

endmodule
