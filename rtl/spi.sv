
// Implement an SPI slave to communicate with the microcontroller.

// It may be possible to use the hard SPI in the iCE40UP5K to do what
// I want, but I'm implementing a soft version first
// (for simulation if nothing else).

module spi (
    input i_Clock,

    input logic i_SampleReady,
    input logic [15:0] i_SampleToOutput,

    output logic o_RegisterWriteEnable,
    output logic [14:0] o_RegisterWriteNumber,
    output logic [15:0] o_RegisterWriteValue,

    // The SPI SCK must be 8x slower than the internal clock!
    input i_SPI_SCK,
    input i_SPI_MOSI,
    output o_SPI_MISO

);

logic r_SPI_SCK_last;
logic w_SPI_SCK_rising;
assign w_SPI_SCK_rising = ! r_SPI_SCK_last && i_SPI_SCK;

logic [15:0] r_SampleToOutput;

logic [31:0] r_InputBuffer;
assign o_RegisterWriteEnable = r_InputBuffer[31];
assign o_RegisterWriteNumber = r_InputBuffer[30:16];
assign o_RegisterWriteValue = r_InputBuffer[15:0];

integer i;
always_ff @ (posedge i_Clock) begin

    if (i_SampleReady)
        r_SampleToOutput <= i_SampleToOutput;

    r_SPI_SCK_last <= i_SPI_SCK;
    if (w_SPI_SCK_rising) begin

        // if (i_SPI_MOSI)
        // $display("i_SPI_MOSI = %d", i_SPI_MOSI);

        // Output sample MSB first
        o_SPI_MISO <= r_SampleToOutput[15];
        if ( ! i_SampleReady) begin
            r_SampleToOutput[0] <= r_SampleToOutput[15];
            for (i = 1; i <= 15; i++)
                r_SampleToOutput[i] <= r_SampleToOutput[i - 1];
        end

        // Input the register number MSB first (i.e. the 1 indicating the register write)
        // The register value LSB will be the last bit
        r_InputBuffer[0] <= i_SPI_MOSI;
        if (o_RegisterWriteEnable) begin
            $display("Last wrote %x to register %x", o_RegisterWriteValue, o_RegisterWriteNumber);
            r_InputBuffer[31:1] <= 0;

        end
        else begin
            for (i = 1; i <= 31; i++)
                r_InputBuffer[i] <= r_InputBuffer[i - 1];

            // o_RegisterWriteValue[0] <= i_SPI_MOSI;
            // for (i = 1; i <= 15; i++)
            //     o_RegisterWriteValue[i] <= o_RegisterWriteValue[i - 1];

            // o_RegisterWriteNumber[0] <= o_RegisterWriteValue[15];
            // for (i = 1; i <= 14; i++)
            //     o_RegisterWriteNumber[i] <= o_RegisterWriteNumber[i - 1];
            // o_RegisterWriteEnable <= o_RegisterWriteNumber[14];

        end

        if (o_RegisterWriteEnable) begin
            // $display("o_RegisterWriteNumber = %x ( %b)", o_RegisterWriteNumber, o_RegisterWriteNumber);
        end



    end
end

endmodule
