
#include "synth.hpp"


// TODO: Find a better way
Synth::Synth() :
    m_Synth {},
    m_SampleBuffer {},
    m_SampleCounter {0},
    m_DataFile { fopen("data.csv", "w") },
    m_t {0.0}
{
}

Synth::~Synth()
{
    fclose(m_DataFile);
}

#include <cmath>
void Synth::tick()
{
    m_Synth.i_Clock = 0;
    m_Synth.eval();
    m_Synth.i_Clock = 1;
    m_Synth.eval();

    if (m_Synth.o_SampleReady)
    {
        int16_t sample = m_Synth.o_Sample;
        m_SampleBuffer.push_front(sample);

        // TODO: Generate an "expected" sample as well
        int16_t expectedSample = static_cast<int16_t>(
            0x7fff *
            std::sin(
                2.0 * M_PI * 440.0 * m_t
                + std::sin(2.0 * M_PI * 880.0 * m_t)
            )
            / 16.0
        );

        fprintf(m_DataFile, "%zu,%d,%d\n", m_SampleCounter++, sample, expectedSample);
    }

    // We execute 256 clock cycles for each audio sample (at 44.1 kHz),
    // so our running clock frequency is 11.2896 MHz (period = 88.577 ns).
    // Note that adding additional operators, voices, or increasing the
    // audio sampling rate will increase the running frequency.
    m_t += 1.0 / (256 * 44.1e3);
}


void Synth::reset()
{
    m_Synth.i_Reset = 1;
    tick();
    m_Synth.i_Reset = 0;

    m_t = 0.0;
}


void Synth::writeRegister(uint16_t registerNumber, uint8_t value)
{
    m_Synth.i_RegisterNumber = registerNumber;
    m_Synth.i_RegisterValue = value;
    m_Synth.i_RegisterWriteEnable = 1;
    tick();
    m_Synth.i_RegisterWriteEnable = 0;
}


void Synth::writeSampleBytes(uint8_t* pRawStream, size_t number)
{
    int16_t* pStream = reinterpret_cast<int16_t*>(pRawStream);
    size_t samplesNeeded = number / sizeof(pStream[0]);
    while (m_SampleBuffer.size() < samplesNeeded)
    {
        // Instead of producing samples on demand
        // (since we can't do it in realtime anyway)
        // we'll just send silence if there aren't any pre-rendered samples.
        m_SampleBuffer.push_front(0);
    }

    for (size_t i = 0; i < samplesNeeded; i++)
    {
        pStream[i] = m_SampleBuffer.front();
        m_SampleBuffer.pop_front();
    }
}


void Synth::writeOperatorRegister(uint8_t voiceNum, uint8_t operatorNum, uint8_t parameter, uint8_t value)
{
    const uint16_t registerNumber = (0b11 << 14) | (parameter << 8) | (operatorNum << 5) | voiceNum;
    writeRegister(registerNumber, value);
}


void Synth::writeVoiceRegister(uint8_t voiceNum, uint8_t parameter, uint8_t value)
{
    const uint16_t registerNumber = (0b10 << 14) | (parameter << 8) | voiceNum;
    writeRegister(registerNumber, value);
}