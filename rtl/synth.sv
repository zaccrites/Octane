
module synth (
    input i_Clock,
    input i_Reset,

    // input logic signed [17:0] i_Operand1,
    // input logic signed [17:0] i_Operand2,
    // output logic signed [17:0] o_Result,




    // Frequency in radians per second
    input logic [17:0] i_Frequency,





    // TODO: Use PWM instead of outputting the full sample
    output [18:0] o_Sample,


    // Sign extend in Verilog because it's easier than C++ :)
    output [31:0] o_Sample32
);
assign o_Sample32 = {{13{o_Sample[17]}}, o_Sample};








// May have to adjust this, depending on how often it is incremented.
// At 2^16 Hz, it will roll over in 4 seconds.
logic [17:0] r_TimeReg;



// TODO: Not sure what to do about the unused bits. The multiplier outputs
// 18, but sine table only can use 13. Just ignore the higher bits directly
// inside the multiplier module?
//
// verilator lint_off UNUSED
logic [17:0] w_SineArg;
logic [31:0] r_PhaseAcc;
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
    // .i_Argument (r_TimeReg),

    // .i_Argument (w_SineArg[9:0]),
    // .i_Argument ({5'b0, w_SineArg[7:0]}),
    // .i_Argument (w_SineArg[17:5]),

    .i_Argument(r_PhaseAcc[31:19]),

    .o_Result   (o_Sample)
);



// PhaseStep = 2^N * Freq_hz / Fs_hz
//
// N=32, so shift left 32
// Fs=2^16, so shift right 16
// Overall, shift left 16
logic [31:0] w_PhaseStep;
assign w_PhaseStep = {i_Frequency, 14'b0};
// TODO: To support fractional frequency



always_ff @ (posedge i_Clock) begin
    if (i_Reset) begin
        r_TimeReg <= 0;
        r_PhaseAcc <= 0;
    end
    else begin

        // TODO: Divide the clock? This should be done at audio frequency.
        r_TimeReg <= r_TimeReg + 1;




        r_PhaseAcc <= r_PhaseAcc + w_PhaseStep;

    end


    // $display("w_SineArg = %d, [7:0] is %d", w_SineArg, w_SineArg[7:0]);

end


endmodule
