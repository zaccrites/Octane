
`ifndef SYNTH_SVH
`define SYNTH_SVH


typedef struct packed
{
    // OP8 can never be a modulator, so we don't keep a bit for it.
    logic [6:0] ModulateWithOP;
    logic IsACarrier;
} AlgorithmWord_t;




`define NUM_VOICES            32
`define NUM_OPERATORS         8
`define NUM_VOICE_OPERATORS   (`NUM_VOICES * `NUM_OPERATORS)


typedef logic unsigned [7:0] VoiceOperatorID_t;
typedef logic unsigned [2:0] OperatorID_t;
typedef logic unsigned [4:0] VoiceID_t;


function OperatorID_t getOperatorID (
    // verilator lint_off UNUSED
    VoiceOperatorID_t voiceOperator
    // verilator lint_on UNUSED
);
    return voiceOperator[7:5];
endfunction


function VoiceID_t getVoiceID (
    // verilator lint_off UNUSED
    VoiceOperatorID_t voiceOperator
    // verilator lint_on UNUSED
);
    return voiceOperator[4:0];
endfunction



`endif
