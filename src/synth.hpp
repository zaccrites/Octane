
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

    void tick();
    void reset();
    void writeRegister(uint16_t registerNumber, uint8_t registerValue);

    // TODO: Move to Python
    void writeOperatorRegister(uint8_t voiceNum, uint8_t operatorNum, uint8_t parameter, uint8_t value);
    void writeVoiceRegister(uint8_t voiceNum, uint8_t parameter, uint8_t value);

    void writeSampleBytes(uint8_t* pRawStream, size_t number);
    const std::deque<int16_t>& getSampleBuffer() const
    {
        return m_SampleBuffer;
    }

public:
    static const uint16_t VOICE_PARAM_KEYON  { 0x00 };
    static const uint16_t VOICE_PARAM_ALGORITHM  { 0x01 };
    // static const uint16_t VOICE_PARAM_AMPLITUDE_ADJUST { 0x02 };

    static const uint16_t OP_PARAM_PHASE_STEP_HIGH  { 0x00 };
    static const uint16_t OP_PARAM_PHASE_STEP_LOW  { 0x01 };
    static const uint16_t OP_PARAM_WAVEFORM_HIGH    { 0x02 };
    static const uint16_t OP_PARAM_WAVEFORM_LOW    { 0x03 };

    // static const uint16_t OP_PARAM_ATTACK_LEVEL  { 0x03 };
    // static const uint16_t OP_PARAM_SUSTAIN_LEVEL  { 0x04 };
    // static const uint16_t OP_PARAM_ATTACK_RATE  { 0x05 };
    // static const uint16_t OP_PARAM_DECAY_RATE  { 0x06 };
    // static const uint16_t OP_PARAM_RELEASE_RATE  { 0x07 };


    // static const uint16_t OP_WAVEFORM_SINE  { 0x0000 };
    // static const uint16_t OP_WAVEFORM_SQUARE { 0x0001 };

private:
    Vsynth m_Synth;
    std::deque<int16_t> m_SampleBuffer;

    size_t m_SampleCounter;
    FILE* m_DataFile;


    /// Time elapsed since last reset
    double m_t;

};


#endif
