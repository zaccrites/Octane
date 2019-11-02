
`ifndef SYNTH_SVH
`define SYNTH_SVH


typedef logic [10:0] AlgorithmWord_t;

function automatic logic getModulateWithOP;
    input AlgorithmWord_t word;
    input integer opNum;
    // getModulateWithOP = word[10:4];
    assert(0 <= opNum && opNum <= 6);
    getModulateWithOP = word[4 + opNum];
endfunction

function automatic logic unsigned [2:0] getNumCarriers;
    // verilator lint_off UNUSED
    input AlgorithmWord_t word;
    // verilator lint_on UNUSED
    getNumCarriers = word[3:1];
endfunction

function automatic logic getIsCarrier;
    // verilator lint_off UNUSED
    input AlgorithmWord_t word;
    // verilator lint_on UNUSED
    getIsCarrier = word[0];
endfunction


`define NUM_VOICES            32
`define NUM_OPERATORS         8
`define NUM_VOICE_OPERATORS   (`NUM_VOICES * `NUM_OPERATORS)


typedef logic unsigned [7:0] VoiceOperatorID_t;
typedef logic unsigned [2:0] OperatorID_t;
typedef logic unsigned [4:0] VoiceID_t;


function automatic OperatorID_t getOperatorID;
    // verilator lint_off UNUSED
    input VoiceOperatorID_t voiceOperator;
    // verilator lint_on UNUSED
begin
    getOperatorID = voiceOperator[7:5];
end
endfunction


function automatic VoiceID_t getVoiceID;
    // verilator lint_off UNUSED
    input VoiceOperatorID_t voiceOperator;
    // verilator lint_on UNUSED
begin
    getVoiceID = voiceOperator[4:0];
end
endfunction


function automatic VoiceOperatorID_t makeVoiceOperatorID;
    input VoiceID_t voice;
    input OperatorID_t operator;
begin
    return {operator, voice};
end
endfunction



`endif
