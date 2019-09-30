
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

    // TODO: Use 8 bits instead?
    logic unsigned [15:0] AttackLevel;
    logic unsigned [15:0] SustainLevel;
    logic unsigned [15:0] AttackRate;
    logic unsigned [15:0] DecayRate;
    logic unsigned [15:0] ReleaseRate;

} OperatorConfig_t;


typedef struct packed
{
    logic KeyOn;
    logic [5:0] Algorithm;

    // Used to reduce per-carrier amplitude when there are multiple
    // carrier operators in the selected algorithm.
    logic unsigned [15:0] AmplitudeAdjust;

    // TODO: KeyOn time for e.g. envelopes

    OperatorConfig_t [5:0] OperatorConfigs;

} VoiceConfig_t;


typedef struct packed
{
    VoiceConfig_t [15:0] VoiceConfigs;

} CoreConfig_t;


`endif
