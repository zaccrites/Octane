
// Implement an SPI slave to communicate with the microcontroller.

// It may be possible to use the hard SPI in the iCE40UP5K to do what
// I want, but I'm implementing a soft version first
// (for simulation if nothing else).

module spi (
    input i_Clock,

    // verilator lint_off UNUSED
    input i_Reset,
    // verilator lint_on UNUSED

    input logic i_SampleReady,
    input logic [15:0] i_SampleToOutput,

    output logic o_RegisterWriteEnable,
    output logic [15:0] o_RegisterWriteNumber,
    output logic [15:0] o_RegisterWriteValue,

    // The SPI SCK must be 8x slower than the internal clock!
    input logic i_SPI_NSS,
    input logic i_SPI_SCK,
    input logic i_SPI_MOSI,
    output logic o_SPI_MISO

);


logic r_SPI_SCK_last;
logic w_SPI_SCK_falling;
// logic w_SPI_Sync;
always_ff @ (posedge i_Clock) r_SPI_SCK_last <= i_SPI_SCK;
assign w_SPI_SCK_falling = r_SPI_SCK_last && ! i_SPI_SCK;
// assign w_SPI_Sync = w_SPI_SCK_falling && i_SPI_NSS;

logic [15:0] r_NextSample;
logic [15:0] r_CurrentSample;

// logic [14:0] r_InputBuffer;
logic [31:0] r_CommandBuffer;
assign o_RegisterWriteEnable = w_CommandComplete;
assign o_RegisterWriteNumber = r_CommandBuffer[31:16];
assign o_RegisterWriteValue = r_CommandBuffer[15:0];



// Counts from 0 to 31 to indicate when a command has finished.
logic [4:0] r_CommandProgressCounter;

logic w_CommandComplete;
assign w_CommandComplete = r_CommandBuffer == 31;


always_ff @ (posedge i_Clock) begin

    // TODO: Reset

    if (i_SampleReady) begin
        r_NextSample <= i_SampleToOutput;
    end

    // If NSS goes high, wipe out the buffers
    if (i_SPI_NSS) begin
        r_CommandBuffer <= 0;
        r_CommandProgressCounter <= 0;
        r_CurrentSample <= r_NextSample;

    end
    else if (w_SPI_SCK_falling) begin
        r_CommandProgressCounter <= r_CommandProgressCounter + 1;
        o_SPI_MISO <= r_CurrentSample[15];

        if (w_CommandComplete) begin
            r_CommandBuffer <= {31'b0, i_SPI_MOSI};
            r_CurrentSample <= r_NextSample;
        end
        else begin
            r_CommandBuffer <= {r_CommandBuffer[30:0], i_SPI_MOSI};
            r_CurrentSample <= {r_CurrentSample[14:0], r_CurrentSample[15]};
        end

        $display("SCK_FALLING: Command buffer is %b %b", r_CommandBuffer[31:16], r_CommandBuffer[15:0]);
    end

end

endmodule
