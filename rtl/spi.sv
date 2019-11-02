
// Implement an SPI slave to communicate with the microcontroller.

// It may be possible to use the hard SPI in the iCE40UP5K to do what
// I want, but I'm implementing a soft version first
// (for simulation if nothing else).

module spi (
    input i_Clock,
    input i_Reset,

    input logic i_SampleReady,
    input logic [15:0] i_SampleToOutput,

    output logic o_RegisterWriteEnable,
    output logic [14:0] o_RegisterWriteNumber,
    output logic [15:0] o_RegisterWriteValue,

    // The SPI SCK must be 8x slower than the internal clock!
    input logic i_SPI_SCK,
    input logic i_SPI_MOSI,
    output logic o_SPI_MISO

);


logic r_SPI_SCK_last;
logic w_SPI_SCK_rising;
assign w_SPI_SCK_rising = ! r_SPI_SCK_last && i_SPI_SCK;

logic [3:0] r_SPI_TickCounter;
logic [15:0] r_NextSample;
logic [15:0] r_CurrentSample;

logic [31:0] r_InputBuffer;
assign o_RegisterWriteEnable = r_InputBuffer[31];
assign o_RegisterWriteNumber = r_InputBuffer[30:16];
assign o_RegisterWriteValue = r_InputBuffer[15:0];


integer i;
always_ff @ (posedge i_Clock) begin

    if (i_Reset) begin
        r_SPI_TickCounter <= 0;
    end

    if (i_SampleReady) begin
        r_NextSample <= i_SampleToOutput;
    end

    r_SPI_SCK_last <= i_SPI_SCK;
    if (w_SPI_SCK_rising) begin
        if ( ! i_Reset) begin
            r_SPI_TickCounter <= r_SPI_TickCounter + 1;
        end

        // Output MSB first.
        o_SPI_MISO <= r_CurrentSample[15];
        if (r_SPI_TickCounter == 4'b1111) begin
            // Only update the output sample every 16 SPI ticks
            r_CurrentSample <= r_NextSample;
        end
        else begin
            r_CurrentSample[0] <= 0;
            for (i = 1; i <= 15; i++) begin
                r_CurrentSample[i] <= r_CurrentSample[i - 1];
            end
        end

        // Input the register number MSB first (i.e. the 1 indicating the register write)
        // The register value LSB will be the last bit
        r_InputBuffer[0] <= i_SPI_MOSI;
        if (o_RegisterWriteEnable) begin
            r_InputBuffer[31:1] <= 0;
        end
        else begin
            for (i = 1; i <= 31; i++) begin
                r_InputBuffer[i] <= r_InputBuffer[i - 1];
            end
        end

    end
end

endmodule
