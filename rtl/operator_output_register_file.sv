
module operator_output_register_file (
    input logic i_Clock,

    input logic [7:0] i_WriteAddress,
    input logic signed [15:0] i_DataIn,

    input logic [7:0] i_ReadAddress [8],
    output logic signed [15:0] o_DataOut [8]

);

// This implements an 8x read port, 1x write port register file.
// On iCE40 this means that eight block RAMs will be needed to duplicate
// the data, with the write ports tied together.
//

logic signed [15:0] r_Registers [3] [256];

always_ff @ (posedge i_Clock) begin
    // Write-enable is always enabled.
    r_Registers[i_WriteAddress] <= i_DataIn;

    o_DataOut[0] <= r_Registers[i_ReadAddress[0]];
    o_DataOut[1] <= r_Registers[i_ReadAddress[1]];
    o_DataOut[2] <= r_Registers[i_ReadAddress[2]];
    o_DataOut[3] <= r_Registers[i_ReadAddress[3]];
    o_DataOut[4] <= r_Registers[i_ReadAddress[4]];
    o_DataOut[5] <= r_Registers[i_ReadAddress[5]];
    o_DataOut[6] <= r_Registers[i_ReadAddress[6]];
    o_DataOut[7] <= r_Registers[i_ReadAddress[7]];
end

endmodule
