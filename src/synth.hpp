
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
    void writeRegister(uint16_t registerNumber, uint16_t registerValue);

    // TODO: Move to Python
    void writeOperatorRegister(uint16_t voiceNum, uint16_t operatorNum, uint16_t parameter, uint16_t value);
    void writeVoiceRegister(uint16_t voiceNum, uint16_t parameter, uint16_t value);

    void writeSampleBytes(uint8_t* pRawStream, size_t number);
    const std::deque<int16_t>& getSampleBuffer() const
    {
        return m_SampleBuffer;
    }


private:
    Vsynth m_Synth;
    std::deque<int16_t> m_SampleBuffer;

    size_t m_SampleCounter;
    FILE* m_DataFile;

};


#endif
