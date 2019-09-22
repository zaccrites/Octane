
module synth (
    input i_Clock,
    input i_Reset,

    // input logic signed [17:0] i_Operand1,
    // input logic signed [17:0] i_Operand2,
    // output logic signed [17:0] o_Result,




    // Frequency in radians per second
    input logic [23:0] i_Frequency,





    // TODO: Use PWM instead of outputting the full sample
    output [18:0] o_Sample,


    // Sign extend in Verilog because it's easier than C++ :)
    output [31:0] o_Sample32
);


assign o_Sample32 = {{13{o_Sample[17]}}, o_Sample};








// May have to adjust this, depending on how often it is incremented.
// At 2^16 Hz, it will roll over in 4 seconds.
// logic [17:0] r_TimeReg;



// TODO: Not sure what to do about the unused bits. The multiplier outputs
// 18, but sine table only can use 13. Just ignore the higher bits directly
// inside the multiplier module?
//
// verilator lint_off UNUSED
logic [23:0] r_PhaseAcc;
// verilator lint_on UNUSED


// multiplier mult0 (
//     .i_Clock   (i_Clock),
//     .i_Operand1(r_TimeReg),
//     .i_Operand2(i_Frequency),
//     .o_Result  (w_SineArg)
// );


sine_function sin0 (
    .i_Clock    (i_Clock),
    // .i_Reset    (i_Reset),
    .i_Argument(r_PhaseAcc[23:11]),
    .o_Result   (o_Sample)
);


// Still not really sure why summing the frequency up works to
// accumulate phase, but this seems to work under the following conditions.
// I suspect there is additional logic required when these don't line up
// as well.
//
// - Phase accumulator: 24 bits
// - Frequency values: 24 bits
// - Frequency values are multiples of 2^-8 Hz
// - Sample frequency is 2^16 Hz
//
logic [23:0] w_PhaseStep;
assign w_PhaseStep = i_Frequency;




always_ff @ (posedge i_Clock) begin
    if (i_Reset) begin
        r_PhaseAcc <= 0;
    end
    else begin
        r_PhaseAcc <= r_PhaseAcc + w_PhaseStep;
    end
end


endmodule
