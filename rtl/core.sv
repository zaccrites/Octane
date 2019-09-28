
`include "core.svh"


module core (
    input logic i_Clock,
    input logic i_Reset,

    // Should these be inputs? Or handled internally via registers?
    // input logic unsigned [15:0] i_PhaseStep,

    input CoreConfig_t i_Config,

    // verilator lint_off UNUSED
    input logic signed [15:0] i_EnvelopeLevel,
    // verilator lint_on UNUSED

    output logic signed [15:0] o_Subsample,
    output logic o_SubsampleReady,
    output logic o_SampleReady
);












// Global registers
logic unsigned [6:0] r_CycleCounter;
logic unsigned [2:0] w_OperatorNum;
logic unsigned [3:0] w_VoiceNum;
assign w_OperatorNum = r_CycleCounter[6:4];
assign w_VoiceNum = r_CycleCounter[3:0];



// Phase must be accumulated independently for each operator for each voice.
logic unsigned [15:0] r_PhaseAcc [95:0];

// Pipeline registers
// Bottom few bits of r_Phase not directly used
// verilator lint_off UNUSED
logic unsigned [15:0] r_Phase;
// verilator lint_on UNUSED
logic signed [15:0] r_Subsample [6:0];


`include "core.svh"


// TODO: The product output is 32 bits, but we only take the top 16 bits at the output
// logic signed [15:0] r_SineProduct[2:0];
// verilator lint_off UNUSED
logic signed [31:0] r_SineProduct[2:0];
// verilator lint_on UNUSED



logic signed [15:0] w_SinResult;
sine_function sine (
    .i_Clock (i_Clock),
    .i_Phase (r_Phase[15:3]),
    .o_Result(w_SinResult)
);


// TODO: Verify that the pipeline is set up correctly for this
logic unsigned [15:0] w_ModulationPhase;
assign w_ModulationPhase = 0;


logic unsigned [15:0] w_PhaseStep;
always_comb begin
    // w_PhaseStep = w_VoiceNum[0] ? 654 : 1308;
    // w_PhaseStep = w_VoiceNum[0] ? 654 : 520;

    w_PhaseStep = i_Config.VoiceConfigs[w_VoiceNum].OperatorConfigs[w_OperatorNum].PhaseStep;
end


always_ff @ (posedge i_Clock) begin
    // NOTE: With 8 operators, this wouldn't be necessary. The counter could just overflow.
    // 16 voices * 6 operators = 96 cycles
    // if (i_Reset || r_CycleCounter == 95)
    if (i_Reset || (w_OperatorNum == 5 && w_VoiceNum == 15))
        r_CycleCounter <= 0;
    else
        r_CycleCounter <= r_CycleCounter + 1;

    // Does anything else in here need a reset?

    // TODO: Do we output to an external unit every 16 cycles which
    // combines the samples into a single sample to output at the 96th cycle?
    //
    // A subsample is ready for each voice during the last operator
    // A sample is ready when the last subsample is ready
    o_SubsampleReady <= w_OperatorNum == 5;
    o_SampleReady <= (w_OperatorNum == 5) && (w_VoiceNum == 15);

    // Add incoming frequency to phase accumulator
    // TODO: Is this enough? Do I need to do any math? See above comment
    // (one clock cycle [possibly more if I need to do some math])
    r_PhaseAcc[r_CycleCounter] <= r_PhaseAcc[r_CycleCounter] + w_PhaseStep;

    // if (w_VoiceNum == 0) $display("phaseAcc = %d", r_PhaseAcc[w_VoiceNum]);

    // Sum output of phase accumulator and modulation phase
    // (one clock cycle)
    r_Phase <= r_PhaseAcc[r_CycleCounter] + w_ModulationPhase;  // TODO: Does w_Voice need to be pipelined as well? Or should the initial phase value itself be pipelined from the first clock cycle instead?

    // Sine function table lookup (via module above)
    // (three clock cycles)

    // Multiply sine output by scaled envelope level
    // (four clock cycles)

    // TODO: Why is the output from -0.5 and +0.5, instead of -1.0 and +1.0
    // when i_EnvelopeLevel is 0x7fff? Why does it go crazy when given 0xffff,
    // instead of just inverting sign? It seems like it's doing unsigned
    // multiplication instead, even though all parts of the operation are signed.
    //
    // Technically the multiplier should be Some Q0.16 number anyway, so
    // it probably has to do with shifting by all of the bits or one
    // to many bits when taking the output or something.
    r_SineProduct[0] <= w_SinResult * i_EnvelopeLevel;  // TODO: Pipeline envelope and COM in parallel as well
    r_SineProduct[1] <= r_SineProduct[0];
    r_SineProduct[2] <= r_SineProduct[1];
    r_Subsample[0] <= r_SineProduct[2][31:16];

    // Stall for 8 clock cycles
    r_Subsample[1] <= r_Subsample[0];
    r_Subsample[2] <= r_Subsample[1];
    r_Subsample[3] <= r_Subsample[2];
    r_Subsample[4] <= r_Subsample[3];
    r_Subsample[5] <= r_Subsample[4];
    r_Subsample[6] <= r_Subsample[5];
    o_Subsample    <= r_Subsample[6];

end

endmodule
