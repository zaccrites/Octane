
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

    void writeOperatorRegister(uint8_t voiceNum, uint8_t operatorNum, uint8_t parameter, uint16_t value);
    void writeVoiceRegister(uint8_t voiceNum, uint8_t parameter, uint16_t value);
    void writeGlobalRegister(uint8_t parameter, uint16_t value);

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

    static const uint8_t OP_PARAM_PHASE_STEP  { 0x00 };
    static const uint8_t OP_PARAM_ALGORITHM      { 0x01 };

    static const uint8_t OP_PARAM_ENVELOPE_ATTACK_LEVEL   { 0x02 };
    static const uint8_t OP_PARAM_ENVELOPE_SUSTAIN_LEVEL  { 0x03 };
    static const uint8_t OP_PARAM_ENVELOPE_ATTACK_RATE    { 0x04 };
    static const uint8_t OP_PARAM_ENVELOPE_DECAY_RATE     { 0x05 };
    static const uint8_t OP_PARAM_ENVELOPE_RELEASE_RATE   { 0x06 };


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
