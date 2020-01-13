
`ifndef SYNTH_SVH
`define SYNTH_SVH


// typedef logic [10:0] AlgorithmWord_t;
`define ALGORITHM_WORD  [10:0]

function automatic getModulateWithOP;
    input `ALGORITHM_WORD word;
    input integer opNum;
    // assert(0 <= opNum && opNum <= 6);
    getModulateWithOP = word[4 + opNum];
endfunction

function automatic [2:0] getNumCarriers;
    // verilator lint_off UNUSED
    input `ALGORITHM_WORD word;
    // verilator lint_on UNUSED
    getNumCarriers = word[3:1];
endfunction

function automatic getIsCarrier;
    // verilator lint_off UNUSED
    input `ALGORITHM_WORD word;
    // verilator lint_on UNUSED
    getIsCarrier = word[0];
endfunction


`define NUM_VOICES            32
`define NUM_OPERATORS         8
`define NUM_VOICE_OPERATORS   (`NUM_VOICES * `NUM_OPERATORS)


`define VOICE_OPERATOR_ID [7:0]
`define OPERATOR_ID [2:0]
`define VOICE_ID [4:0]


function automatic `OPERATOR_ID getOperatorID;
    // verilator lint_off UNUSED
    input `VOICE_OPERATOR_ID voiceOperator;
    // verilator lint_on UNUSED
begin
    getOperatorID = voiceOperator[7:5];
end
endfunction


function automatic `VOICE_ID getVoiceID;
    // verilator lint_off UNUSED
    input `VOICE_OPERATOR_ID voiceOperator;
    // verilator lint_on UNUSED
begin
    getVoiceID = voiceOperator[4:0];
end
endfunction


function automatic `VOICE_OPERATOR_ID makeVoiceOperatorID;
    input `VOICE_ID voice;
    input `OPERATOR_ID operator;
begin
    makeVoiceOperatorID = {operator, voice};
end
endfunction



`endif
