
module operator (
    input logic i_Clock,
    input logic i_Reset,

    // Should these be inputs? Or handled internally via registers?
    input logic unsigned [15:0] i_PhaseStep,

    // verilator lint_off UNUSED
    input logic signed [15:0] i_EnvelopeLevel,
    // verilator lint_on UNUSED

    output logic signed [15:0] o_Sample,
    output logic o_SampleReady
);












// Global registers
logic unsigned [6:0] r_CycleCounter;
// logic unsigned [2:0] w_OperatorNum;
// logic unsigned [3:0] w_VoiceNum;
// assign w_OperatorNum = r_CycleCounter[6:4];
// assign w_VoiceNum = r_CycleCounter[3:0];



// Phase must be accumulated independently for each operator for each voice.
logic unsigned [15:0] r_PhaseAcc [95:0];

// Pipeline registers
// Bottom few bits of r_Phase not directly used
// verilator lint_off UNUSED
logic unsigned [15:0] r_Phase;
// verilator lint_on UNUSED
logic signed [15:0] r_SineProduct[2:0];
logic signed [15:0] r_Sample [6:0];



logic signed [15:0] w_SinResult;
sine_function sine (
    .i_Clock (i_Clock),
    .i_Phase (r_Phase[15:3]),
    .o_Result(w_SinResult)
);


// TODO: Verify that the pipeline is set up correctly for this
logic unsigned [15:0] w_ModulationPhase;
assign w_ModulationPhase = 0;




always_ff @ (posedge i_Clock) begin
    // NOTE: With 8 operators, this wouldn't be necessary. The counter could just overflow.
    // 16 voices * 6 operators = 96 cycles
    // if (i_Reset || r_CycleCounter == 95)
    if (i_Reset || r_CycleCounter == 96)
        r_CycleCounter <= 0;
    else
        r_CycleCounter <= r_CycleCounter + 1;

    // Does anything else in here need a reset?

    // TODO: Do we output to an external unit every 16 cycles which
    // combines the samples into a single sample to output at the 96th cycle?
    o_SampleReady <= r_CycleCounter == 95;

    // Add incoming frequency to phase accumulator
    // TODO: Is this enough? Do I need to do any math? See above comment
    // (one clock cycle [possibly more if I need to do some math])
    r_PhaseAcc[r_CycleCounter] <= r_PhaseAcc[r_CycleCounter] + i_PhaseStep;

    // if (w_VoiceNum == 0) $display("phaseAcc = %d", r_PhaseAcc[w_VoiceNum]);

    // Sum output of phase accumulator and modulation phase
    // (one clock cycle)
    r_Phase <= r_PhaseAcc[r_CycleCounter] + w_ModulationPhase;  // TODO: Does w_Voice need to be pipelined as well? Or should the initial phase value itself be pipelined from the first clock cycle instead?

    // Sine function table lookup (via module above)
    // (three clock cycles)

    // Multiply sine output by scaled envelope level
    // (four clock cycles)
    // r_SineProduct[0] <= w_SinResult * i_EnvelopeLevel;  // TODO: Pipeline envelope and COM in parallel as well
    r_SineProduct[0] <= w_SinResult;  // TODO: Pipeline envelope and COM in parallel as well
    r_SineProduct[1] <= r_SineProduct[0];
    r_SineProduct[2] <= r_SineProduct[1];
    r_Sample[0] <= r_SineProduct[2];

    // Stall for 8 clock cycles
    r_Sample[1] <= r_Sample[0];
    r_Sample[2] <= r_Sample[1];
    r_Sample[3] <= r_Sample[2];
    r_Sample[4] <= r_Sample[3];
    r_Sample[5] <= r_Sample[4];
    r_Sample[6] <= r_Sample[5];
    o_Sample    <= r_Sample[6];

end

endmodule
