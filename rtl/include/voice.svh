
`include "operator.svh"

`ifndef VOICE_SVH
`define VOICE_SVH


typedef struct packed
{

    logic KeyOn;

    // Two operator only for now
    // 0: Op2 is modulated by Op1 (FM synth)
    // 1: Op1 and Op2 are added together (additive synth)
    logic Algorithm;

    OperatorRegisters_t [2:1] Operator;


} VoiceRegisters_t;


`endif
