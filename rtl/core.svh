
`ifndef CORE_SVH
`define CORE_SVH


`define SEL_NONE                3'd0
`define SEL_PREVIOUS            3'd1
`define SEL_MREG                3'd2
`define SEL_MREG_PLUS_PREVIOUS  3'd3
`define SEL_FREG                3'd4


typedef struct packed
{
    logic unsigned [15:0] PhaseStep;
    logic Waveform;

    // TODO: Use a real envelope generator
    logic unsigned [15:0] EnvelopeLevel;

} OperatorConfig_t;


typedef struct packed
{
    logic KeyOn;
    logic [5:0] Algorithm;

    // TODO: KeyOn time for e.g. envelopes

    OperatorConfig_t [5:0] OperatorConfigs;

} VoiceConfig_t;


typedef struct packed
{
    VoiceConfig_t [15:0] VoiceConfigs;

} CoreConfig_t;


`endif
