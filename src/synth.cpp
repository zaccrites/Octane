
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

    // We execute 96 clock cycles for each audio sample (at 44.1 kHz),
    // so our running clock frequency is 4.2336 MHz (period = 232.6 ns).
    // Note that adding additional operators, voices, or increasing the
    // audio sampling rate will increase the running frequency.
    m_t += 1.0 / (96 * 44.1e3);
}


void Synth::reset()
{
    m_Synth.i_Reset = 1;
    tick();
    m_Synth.i_Reset = 0;

    m_t = 0.0;
}


void Synth::writeRegister(uint16_t registerNumber, uint16_t value)
{
    m_Synth.i_RegisterNumber = registerNumber;
    m_Synth.i_RegisterValue = value;
    m_Synth.i_RegisterWriteEnable = 1;
    tick();
    m_Synth.i_RegisterWriteEnable = 0;
}

#include <chrono>
void Synth::writeSampleBytes(uint8_t* pRawStream, size_t number)
{
    auto startTime = std::chrono::system_clock::now();

    int16_t* pStream = reinterpret_cast<int16_t*>(pRawStream);
    size_t samplesNeeded = number / sizeof(pStream[0]);
    while (m_SampleBuffer.size() < samplesNeeded)
    {
        tick();
    }

    for (size_t i = 0; i < samplesNeeded; i++)
    {
        pStream[i] = m_SampleBuffer.front();
        m_SampleBuffer.pop_front();
    }

    auto endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> duration = endTime - startTime;
    double actualMs = duration.count() * 1000.0;
    double maxMs = static_cast<double>(samplesNeeded) / 44100.0;

    // printf("It took %.4f ms to get %zu samples (%.1f%% of available time)\n",
    //     actualMs,
    //     samplesNeeded,
    //     actualMs / maxMs);
}


void Synth::writeOperatorRegister(uint16_t voiceNum, uint16_t operatorNum, uint16_t parameter, uint16_t value)
{
    // FUTURE: This may need to be reworked if I ever want e.g.
    // 8 operators and 32 voices, as Voice-level configuration
    // won't be able to go in operator slot 0 any more with only 3 bits.
    const uint16_t registerNumber =
        ((voiceNum & 0x001f) << 11) |        // 5 bit voice
        ((operatorNum & 0x0007) << 8) |      // 3 bit operator
        (parameter & 0x00ff);                // 8 bit parameter
    writeRegister(registerNumber, value);
}


void Synth::writeVoiceRegister(uint16_t voiceNum, uint16_t parameter, uint16_t value)
{
    writeOperatorRegister(voiceNum, 0, parameter, value);
}
