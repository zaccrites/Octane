
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
    input logic i_SPI_NSS,
    input logic i_SPI_SCK,
    input logic i_SPI_MOSI,
    output logic o_SPI_MISO

);


logic r_SPI_SCK_last;
logic w_SPI_SCK_falling;
logic w_SPI_Sync;
assign w_SPI_SCK_falling = r_SPI_SCK_last && ! i_SPI_SCK;
assign w_SPI_Sync = w_SPI_SCK_falling && i_SPI_NSS;

logic [15:0] r_NextSample;
logic [15:0] r_CurrentSample;

logic [31:0] r_InputBuffer;
assign o_RegisterWriteEnable = r_InputBuffer[31];
assign o_RegisterWriteNumber = r_InputBuffer[30:16];
assign o_RegisterWriteValue = r_InputBuffer[15:0];


always_ff @ (posedge i_Clock) begin

    if (i_Reset) begin
        r_InputBuffer <= 0;
    end

    if (i_SampleReady) begin
        r_NextSample <= i_SampleToOutput;
    end

    r_SPI_SCK_last <= i_SPI_SCK;
    if (w_SPI_SCK_falling) begin

        // $display("Input buffer is %b %b", r_InputBuffer[31:16], r_InputBuffer[15:0]);

        if (w_SPI_Sync) begin
            // $display("sync");
            // Do I need additional synchronization? Possibly by "writing"
            // to a dummy sentinel register?

            // ugh
            if ( ! i_Reset) begin
                r_InputBuffer[31:1] <= r_InputBuffer[31] ? 0 : r_InputBuffer[30:0];
            end
            r_CurrentSample <= {r_NextSample[14:0], 1'b0};
            // $display("Will next output %d (0x%04x) (0b%b)", $signed(r_NextSample), r_NextSample, r_NextSample);
        end
        else begin
            if ( ! i_Reset) r_InputBuffer[31:1] <= r_InputBuffer[30:0];
            r_CurrentSample <= {r_CurrentSample[14:0], 1'b0};
        end


        // Input MSB first
        if ( ! i_Reset) r_InputBuffer[0] <= i_SPI_MOSI;

        // Output MSB first.
        if (w_SPI_Sync) begin
            o_SPI_MISO <= r_NextSample[15];
            // $display("Outputting bit %d", r_NextSample[15]);
        end
        else begin
            o_SPI_MISO <= r_CurrentSample[15];
            // $display("Outputting bit %d", r_CurrentSample[15]);
        end




        // if (w_SPI_Sync) begin
        //     if (r_InputBuffer[31] && ! i_Reset) r_InputBuffer[31:1] <= 0;
        //     r_CurrentSample <= r_NextSample;
        // end
        // else begin
        //     if ( ! i_Reset) r_InputBuffer[31:1] <= r_InputBuffer[30:0];
        //     r_CurrentSample <= {r_CurrentSample[14:0], 1'b0};
        // end

    end
end

endmodule
