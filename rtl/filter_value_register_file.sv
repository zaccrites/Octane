
// TODO: Need to rename this and operator_output_register_file
module filter_value_register_file (
    input logic i_Clock,

    input logic [7:0] i_FilterReadAddress [3],
    output logic signed [15:0] o_FilterDataOut [3]
);


always_ff @ (posedge i_Clock) begin

    // TODO

    // Every time we do a filter operation on the most recent output
    // of the operator (sample n), we need to save that value to another
    // BRAM on the next clock (where it becomes sample n-1), and then again
    // on the clock after that (whete it becomes sample n-2).
    //
    // I think that since I have to read each of these block RAMs to do the
    // filter DSP anyway, I can just route the output of the first two BRAMs
    // to the input of the next two, where they will be latched on the next
    // clock.

end

endmodule
