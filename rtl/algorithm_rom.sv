
module algorithm_rom (
    input logic i_Clock,
    input logic [5:0] i_Algorithm,
    input logic [2:0] i_Operator,

    output logic [2:0] o_SEL,
    output logic o_MREN,
    output logic o_FREN
);

logic [4:0] r_ROM [512];
initial $readmemh("rtl/algorithm_rom.hex", r_ROM);

logic [4:0] r_ControlWord;
assign o_FREN = r_ControlWord[0];
assign o_MREN = r_ControlWord[1];
assign o_SEL = r_ControlWord[4:2];

logic [8:0] w_Index;
assign w_Index = {i_Algorithm, i_Operator};

always_ff @ (posedge i_Clock) begin
    r_ControlWord <= r_ROM[w_Index];
end


endmodule
