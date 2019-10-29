
#include "synth.hpp"

#include <algorithm>  // for std::fill_n


// TODO: Find a better way
Synth::Synth() :
    m_Synth {},
    m_SampleBuffer {},
    m_SampleCounter {0},
    m_DataFile { fopen("data.csv", "w") },
    m_NoteOnState {},
    m_t {0.0}
{
    std::fill_n(m_NoteOnState, 32, false);
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

        // // TODO: Generate an "expected" sample as well
        // int16_t expectedSample = static_cast<int16_t>(
        //     0x7fff *
        //     std::sin(
        //         2.0 * M_PI * 440.0 * m_t
        //         + std::sin(2.0 * M_PI * 880.0 * m_t)
        //     )
        //     / 16.0
        // );
        // fprintf(m_DataFile, "%zu,%d,%d\n", m_SampleCounter++, sample, expectedSample);

        fprintf(m_DataFile, "%zu,%d\n", m_SampleCounter++, sample);

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


void Synth::setNoteOn(uint8_t voiceNum, bool noteOn)
{
    voiceNum = voiceNum % 32;
    if (m_NoteOnState[voiceNum] == noteOn)
    {
        return;
    }
    m_NoteOnState[voiceNum] = noteOn;

    const uint8_t bank = voiceNum / 16;
    uint16_t newRegisterValue = 0;
    for (int i = 0; i < 16; i++)
    {
        newRegisterValue |= static_cast<uint16_t>(m_NoteOnState[16 * bank + i]) << i;
    }

    if (bank == 0)
    {
        writeGlobalRegister(GLOBAL_PARAM_NOTEON_BANK0, newRegisterValue);
    }
    else
    {
        writeGlobalRegister(GLOBAL_PARAM_NOTEON_BANK1, newRegisterValue);
    }
}


void Synth::writeRegister(uint16_t registerNumber, uint16_t value)
{
    m_Synth.i_RegisterWriteNumber = registerNumber;
    m_Synth.i_RegisterWriteValue = value;
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


void Synth::writeOperatorRegister(uint8_t voiceNum, uint8_t operatorNum, uint8_t parameter, uint16_t value)
{
    const uint16_t registerNumber = (0b11 << 14) | (parameter << 8) | (operatorNum << 5) | voiceNum;
    writeRegister(registerNumber, value);
}


void Synth::writeVoiceRegister(uint8_t voiceNum, uint8_t parameter, uint16_t value)
{
    const uint16_t registerNumber = (0b10 << 14) | (parameter << 8) | voiceNum;
    writeRegister(registerNumber, value);
}


void Synth::writeGlobalRegister(uint8_t parameter, uint16_t value)
{
    const uint16_t registerNumber = (0b01 << 14) | (parameter << 8);
    writeRegister(registerNumber, value);
}
