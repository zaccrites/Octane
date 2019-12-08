
// Double register an input to synchronize it to the FPGA clock domain

module double_register (
    input i_Clock,
    input logic i_Input,
    output logic o_Output
);

logic r_Register;

always_ff @ (posedge i_Clock) begin
    r_Register <= i_Input;
    o_Output <= r_Register;
end

endmodule
