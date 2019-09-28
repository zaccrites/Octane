
`ifndef CORE_SVH
`define CORE_SVH


typedef struct packed
{
    logic unsigned [15:0] PhaseStep;
    logic Waveform;

} OperatorConfig_t;


typedef struct packed
{
    logic KeyOn;

    // TODO: KeyOn time for e.g. envelopes

    OperatorConfig_t [5:0] OperatorConfigs;

} VoiceConfig_t;


typedef struct packed
{
    VoiceConfig_t [15:0] VoiceConfigs;

} CoreConfig_t;


`endif
