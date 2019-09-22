
`include "registers.svh"

module operator (
    input logic i_Clock,
    // input logic i_Reset,

    // input OperatorConfig_t i_Config

    // TODO: Not all bits used?
    // verilator lint_off UNUSED
    input signed [23:0] i_Phase,
    // verilator lint_on UNUSED

    // Really only need fractional bits?
    // 8 integer bits, 8 fractional bits
    input signed [15:0] i_AmplitudeFactor,

    output [23:0] o_Output
);


// TODO: Share a sine table between two operators.
// Perhaps the operators can come as pairs?


// TODO: Pre-adder with feedback amplitude


logic signed [18:0] w_SinResult;

sine_function sin0 (
    .i_Clock   (i_Clock),
    .i_Argument(i_Phase[23:11]),
    .o_Result  (w_SinResult)
);


// TODO: Use a multiplication module

// The product is natively 18+24=42 bits

// Need to pass paramaters along with sine table pipeline...
logic signed [15:0] r_AmplitudeFactor [1:0];

// TODO: Not all bits used?
// verilator lint_off UNUSED
// logic signed [39:0] r_Product;
logic signed [34:0] r_Product;

// 16 bits * 19 bits = 35 bit product
// logic signed [34:0] r_Product;
// verilator lint_on UNUSED


// logic signed [18:0] r_SinResult;

always_ff @ (posedge i_Clock) begin
    r_AmplitudeFactor[0] <= i_AmplitudeFactor;
    r_AmplitudeFactor[1] <= r_AmplitudeFactor[0];

    // r_Product <= {w_SinResult, 5'b0} * r_AmplitudeFactor[1];

    // r_SinResult <= w_SinResult;

    r_Product <= w_SinResult * r_AmplitudeFactor[1];
    o_Output <= r_Product[26:3];  // shift right by 8 to counter the 8 fractional bits

    // FUTURE: I noticed a bug at one point where the multiplier had to be increased
    // to 256 to get back to the normal unity amplitude. I thought that this could
    // possibly be made into a feature, since e.g. ADSR envelopes will only ever
    // want to lower the volume (I think), and this would give more precision to
    // do that than 256 steps only (plus I think the use case to increase above
    // unity will be rather limited). It's worth considering for the future.
    //
    // Also remember that the number will potentially have to be adjusted to
    // for multiple mixed voices all adding up to full output signal.


    // o_Output <= {w_SinResult, 5'b0};


    // if (r_SinResult[18] && r_Product != 0)
    //     $display("r_SinResult = %x, r_Product = %x", r_SinResult, r_Product);


    // if (r_AmplitudeFactor[1] != 0)
    //     $display("w_SinResult = %d, r_AmplitudeFactor = %d", {w_SinResult, 5'b0}, r_AmplitudeFactor[1]);

    // if (r_Product != 0)
    //     $display("r_Product is %x, [31:8] is %x", r_Product, r_Product[31:8]);
end



endmodule
