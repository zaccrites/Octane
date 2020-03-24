
#include "synth.hpp"

#include <algorithm>  // for std::fill_n


Synth::Synth() :
    m_Synth {},
    m_SampleBuffer {},
    m_SampleCounter {0},
    m_DataFile { fopen("data.csv", "w") },
    m_NoteOnState {},
    m_CommandQueue {}
{
    std::fill_n(m_NoteOnState, 8, false);
}

Synth::~Synth()
{
    fclose(m_DataFile);
}


void Synth::tick()
{
    m_Synth.i_Clock = 0;
    m_Synth.eval();
    m_Synth.i_Clock = 1;
    m_Synth.eval();
}


void Synth::reset()
{
    m_Synth.i_Reset = 1;
    tick();
    m_Synth.i_Reset = 0;
}


void Synth::setNoteOn(uint8_t voiceNum, bool noteOn)
{
    voiceNum = voiceNum % 8;
    if (m_NoteOnState[voiceNum] == noteOn)
    {
        return;
    }
    m_NoteOnState[voiceNum] = noteOn;

    uint16_t newRegisterValue = 0;
    for (int i = 0; i < 8; i++)
    {
        newRegisterValue |= static_cast<uint16_t>(m_NoteOnState[i]) << i;
    }

    // TODO: This isn't really a "voice op" parameter anymore.
    // Need to reorganize registers now that I need all 14 bits
    // of a "global" address for the sine table.
    // If I can write the whole thing at once then this problem
    // probably goes away (could just have 8 bit addresses, honestly).
    writeOperatorRegister(0, 0, PARAM_NOTEON_BANK0, newRegisterValue);
}


bool Synth::getNoteOn(uint8_t voiceNum) const
{
    return m_NoteOnState[voiceNum % 8];
}


void Synth::sendReceive()
{
    if (m_CommandQueue.empty())
    {
        m_Synth.i_RegisterWriteEnable = 0;
    }
    else
    {
        auto& command = m_CommandQueue.front();
        m_Synth.i_RegisterWriteEnable = 1;
        m_Synth.i_RegisterWriteNumber = command.first;
        m_Synth.i_RegisterWriteValue = command.second;
        m_CommandQueue.pop();
    }
    tick();

    if (m_Synth.o_SampleReady)
    {
        auto sample = static_cast<int16_t>(m_Synth.o_Sample);
        m_SampleBuffer.push_front(sample);
        fprintf(m_DataFile, "%zu,%d\n", ++m_SampleCounter, sample);
    }
}


void Synth::writeRegister(uint16_t registerNumber, uint16_t value)
{
    m_CommandQueue.push({registerNumber, value});
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


// TODO: Extract this to a common file in MCU
#include <cmath>
void Synth::populateSineTable()
{
    const uint16_t TABLE_SIZE = 16 * 1024;
    const uint16_t BIT_DEPTH = 15;
    const uint16_t MAX_RANGE = 1 << BIT_DEPTH;
    const uint16_t MASK = MAX_RANGE - 1;

    for (uint16_t i = 0; i < TABLE_SIZE; i++)
    {
        // https://zipcpu.com/dsp/2017/08/26/quarterwave.html
        const double phase =
            2.0 * M_PI *
            (2.0 * static_cast<double>(i) + 1) /
            (2.0 * static_cast<double>(TABLE_SIZE) * 4.0);

        // https://stackoverflow.com/a/12946226
        const double sineValue = std::sin(phase);
        const uint16_t value = static_cast<uint16_t>(sineValue * MAX_RANGE) & MASK;

        uint16_t registerNumber = (0b11 << 14) | i;
        writeRegister(registerNumber, value);
    }
}


void Synth::writeOperatorRegister(uint8_t voiceNum, uint8_t operatorNum, uint8_t parameter, uint16_t value)
{
    const uint16_t registerNumber = (0b10 << 14) | (parameter << 8) | (operatorNum << 5) | voiceNum;
    writeRegister(registerNumber, value);
}
