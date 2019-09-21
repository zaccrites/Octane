
module synth (
    input i_Clock,
    input i_Reset,

    input logic signed [17:0] i_Operand1,
    input logic signed [17:0] i_Operand2,
    output logic signed [17:0] o_Result,


    input logic [12:0] i_Arg,


    // TODO: Use PWM instead of outputting the full sample
    output [18:0] o_Sample,


    // Sign extend in Verilog because it's easier than C++ :)
    output [31:0] o_Sample32
);



multiplier mult0 (
    .i_Clock   (i_Clock),
    .i_Operand1(i_Operand1),
    .i_Operand2(i_Operand2),
    .o_Result  (o_Result)
);




logic [12:0] r_TimeReg;




sine_function sin0 (
    .i_Clock    (i_Clock),
    // .i_Reset    (i_Reset),
    // .i_Argument (r_TimeReg),
    .i_Argument (i_Arg),
    .o_Result   (o_Sample)
);

assign o_Sample32 = {{13{o_Sample[17]}}, o_Sample};



always_ff @ (posedge i_Clock) begin
    if (i_Reset) begin
        r_TimeReg <= 0;
    end
    else begin

        // TODO: Divide the clock? This should be done at audio frequency.
        r_TimeReg <= r_TimeReg + 1;
    end

end


endmodule
