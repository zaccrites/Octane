
`include "synth.svh"

module stage_filter (
    input i_Clock

    // TODO

);

always_ff @ (posedge i_Clock) begin

    // TODO: All DSP elements will be used here, so create them in a loop
    // to minimize the work required to get multiplication to infer as
    // the sysDSP primitive.
    //
    // Or use `ifdef to instantiate them directly when using Lattice tools.
    //

end

endmodule
