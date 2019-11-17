
// Implement an SPI slave to communicate with the microcontroller.

// It may be possible to use the hard SPI in the iCE40UP5K to do what
// I want, but I'm implementing a soft version first
// (for simulation if nothing else).

module spi (
    input i_Clock,

    input logic i_SampleReady,
    input logic [15:0] i_SampleToOutput,

    output logic o_RegisterWriteEnable,
    output logic [15:0] o_RegisterWriteNumber,
    output logic [15:0] o_RegisterWriteValue,

    // The SPI SCK must be 8x slower than the internal clock!
    input logic i_SPI_CS,
    input logic i_SPI_SCK,
    input logic i_SPI_MOSI,
    output logic o_SPI_MISO

);


logic w_SPI_CS;
double_register SPI_CS_sync (
    .i_Clock (i_Clock),
    .i_Input (i_SPI_CS),
    .o_Output(w_SPI_CS)
);

logic w_SPI_SCK;
double_register SPI_SCK_sync (
    .i_Clock (i_Clock),
    .i_Input (i_SPI_SCK),
    .o_Output(w_SPI_SCK)
);

logic w_SPI_MOSI;
double_register SPI_MOSI_sync (
    .i_Clock (i_Clock),
    .i_Input (i_SPI_MOSI),
    .o_Output(w_SPI_MOSI)
);


logic r_SPI_SCK_last;
always_ff @ (posedge i_Clock) r_SPI_SCK_last <= w_SPI_SCK;
logic w_SPI_SCK_falling;
assign w_SPI_SCK_falling = r_SPI_SCK_last && ! w_SPI_SCK;
logic w_SPI_SCK_rising;
assign w_SPI_SCK_rising = ! r_SPI_SCK_last && w_SPI_SCK;

logic [15:0] r_NextSample;
logic [15:0] r_CurrentSample;



// Use an extra bit to determine when the data has finished writing.
// If the 33rd bit is set, we have 32 valid bits in the other slots.

logic [32:0] r_InputBuffer;
assign o_RegisterWriteEnable = r_InputBuffer[32];
assign o_RegisterWriteNumber = r_InputBuffer[31:16];
assign o_RegisterWriteValue = r_InputBuffer[15:0];

always_ff @ (posedge i_Clock) begin

    if (i_SampleReady) begin
        r_NextSample <= i_SampleToOutput;
    end

    if (w_SPI_CS) begin
        // Clear away any progress (no valid bits yet)
        r_InputBuffer <= 33'b1;

        // Keep next sample current while not active.
        r_CurrentSample <= r_NextSample;
    end
    else if (w_SPI_SCK_falling) begin
        if (r_InputBuffer[32]) begin
            // Start a new command
            r_InputBuffer <= {32'b1, w_SPI_MOSI};

            // New command, new output sample
            r_CurrentSample <= r_NextSample;
        end
        else begin
            // Shift the current command bits
            r_InputBuffer <= {r_InputBuffer[31:0], i_SPI_MOSI};

            // We output the same sample twice for each command
            r_CurrentSample <= {r_CurrentSample[14:0], r_CurrentSample[15]};
        end
    end
    else if (w_SPI_SCK_rising) begin
        // Prepare the output for the next SCK falling edge
        o_SPI_MISO <= r_CurrentSample[15];
    end

end

endmodule
