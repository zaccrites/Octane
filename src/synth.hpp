
#ifndef SYNTH_HPP
#define SYNTH_HPP

#include <stddef.h>  // for size_t
#include <stdint.h>
#include <deque>

#include "Vsynth.h"


class Synth
{
public:

    Synth();
    ~Synth();

    Synth(const Synth&) = delete;
    Synth& operator=(const Synth&) = delete;

    void tick();
    void reset();
    void writeRegister(uint16_t registerNumber, uint8_t registerValue);

    void setNoteOn(uint8_t voiceNum, bool noteOn);

    void writeOperatorRegister(uint8_t voiceNum, uint8_t operatorNum, uint8_t parameter, uint8_t value);
    void writeVoiceRegister(uint8_t voiceNum, uint8_t parameter, uint8_t value);
    void writeGlobalRegister(uint8_t parameter, uint8_t value);

    void writeSampleBytes(uint8_t* pRawStream, size_t number);
    const std::deque<int16_t>& getSampleBuffer() const
    {
        return m_SampleBuffer;
    }

    Vsynth& getRawModel()
    {
        return m_Synth;
    }

public:
    static const uint8_t GLOBAL_PARAM_NOTEON_BANK0  { 0x00 };
    static const uint8_t GLOBAL_PARAM_NOTEON_BANK1  { 0x01 };
    static const uint8_t GLOBAL_PARAM_NOTEON_BANK2  { 0x02 };
    static const uint8_t GLOBAL_PARAM_NOTEON_BANK3  { 0x03 };

    static const uint8_t OP_PARAM_PHASE_STEP_HIGH  { 0x00 };
    static const uint8_t OP_PARAM_PHASE_STEP_LOW  { 0x01 };
    static const uint8_t OP_PARAM_WAVEFORM_HIGH    { 0x02 };
    static const uint8_t OP_PARAM_WAVEFORM_LOW    { 0x03 };

    static const uint8_t OP_PARAM_ENVELOPE_L1    { 0x04 };
    static const uint8_t OP_PARAM_ENVELOPE_L2    { 0x05 };
    static const uint8_t OP_PARAM_ENVELOPE_L3    { 0x06 };
    static const uint8_t OP_PARAM_ENVELOPE_L4    { 0x07 };
    static const uint8_t OP_PARAM_ENVELOPE_R1    { 0x08 };
    static const uint8_t OP_PARAM_ENVELOPE_R2    { 0x09 };
    static const uint8_t OP_PARAM_ENVELOPE_R3    { 0x0a };
    static const uint8_t OP_PARAM_ENVELOPE_R4    { 0x0b };

    static const uint8_t OP_PARAM_ALGORITHM_HIGH      { 0x0c };
    static const uint8_t OP_PARAM_ALGORITHM_LOW      { 0x0d };

    // static const uint8_t OP_WAVEFORM_SINE  { 0x0000 };
    // static const uint8_t OP_WAVEFORM_SQUARE { 0x0001 };

private:
    Vsynth m_Synth;
    std::deque<int16_t> m_SampleBuffer;

    size_t m_SampleCounter;
    FILE* m_DataFile;

    bool m_NoteOnState[32];

    /// Time elapsed since last reset
    double m_t;

};


#endif
