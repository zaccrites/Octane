
#include "synth.hpp"

#include <algorithm>  // for std::fill_n


// TODO: Find a better way
Synth::Synth() :
    m_Synth {},
    m_SampleBuffer {},
    m_SampleCounter {0},
    m_DataFile { fopen("data.csv", "w") },
    m_NoteOnState {}
{
    std::fill_n(m_NoteOnState, 32, false);
}

Synth::~Synth()
{
    fclose(m_DataFile);
}


void Synth::spiTick()
{
    // The FPGA clock runs 8x as fast as the SPI clock
    // in order to avoid missing SCK edges.

    m_Synth.i_SPI_SCK = 0;
    tick();
    tick();

    // Output the MSB first
    // m_Synth.i_SPI_MOSI = m_SPI_OutputBuffer & 0x8000 != 0;
    m_Synth.i_SPI_MOSI = (m_SPI_OutputBuffer & 0x8000) ? 1 : 0;
    // printf("C++ : m_Synth.i_SPI_MOSI = %d \n", m_Synth.i_SPI_MOSI);
    m_SPI_OutputBuffer = m_SPI_OutputBuffer << 1;
    tick();
    tick();

    m_Synth.i_SPI_SCK = 1;
    tick();
    tick();

    // Input the MSB first
    m_SPI_InputBuffer = (m_SPI_InputBuffer << 1) | (m_Synth.o_SPI_MISO != 0);
    tick();
    tick();

    // It takes 256 ticks to make a sample.
    // There are 8 ticks per SPI tick, so we only have to wait 32
    // SPI ticks between collecting samples.
    if (++m_SPI_TickCounter >= 256 / 8)
    {
        m_SPI_TickCounter = 0;

        auto sample = static_cast<int16_t>(m_SPI_InputBuffer);
        m_SampleBuffer.push_front(sample);

        fprintf(m_DataFile, "%zu,%d\n", m_SampleCounter++, sample);
    }
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



void Synth::spiSendReceive()
{
    if (m_SPI_SendQueue.empty())
    {
        m_SPI_OutputBuffer = 0;
    }
    else
    {
        m_SPI_OutputBuffer = m_SPI_SendQueue.front();
        printf("Will output             %04x (%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c)\n", m_SPI_OutputBuffer,
            ((m_SPI_OutputBuffer & 0x8000) ? '1' : '0'), ((m_SPI_OutputBuffer & 0x4000) ? '1' : '0'),
            ((m_SPI_OutputBuffer & 0x2000) ? '1' : '0'), ((m_SPI_OutputBuffer & 0x1000) ? '1' : '0'),
            ((m_SPI_OutputBuffer & 0x0800) ? '1' : '0'), ((m_SPI_OutputBuffer & 0x0400) ? '1' : '0'),
            ((m_SPI_OutputBuffer & 0x0200) ? '1' : '0'), ((m_SPI_OutputBuffer & 0x0100) ? '1' : '0'),
            ((m_SPI_OutputBuffer & 0x0080) ? '1' : '0'), ((m_SPI_OutputBuffer & 0x0040) ? '1' : '0'),
            ((m_SPI_OutputBuffer & 0x0020) ? '1' : '0'), ((m_SPI_OutputBuffer & 0x0010) ? '1' : '0'),
            ((m_SPI_OutputBuffer & 0x0008) ? '1' : '0'), ((m_SPI_OutputBuffer & 0x0004) ? '1' : '0'),
            ((m_SPI_OutputBuffer & 0x0002) ? '1' : '0'), ((m_SPI_OutputBuffer & 0x0001) ? '1' : '0')

        );
        m_SPI_SendQueue.pop();
    }

    for (int i = 0; i < 16; i++) spiTick();
}


void Synth::writeRegister(uint16_t registerNumber, uint16_t value)
{
    m_SPI_SendQueue.push(registerNumber);
    m_SPI_SendQueue.push(value);
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


void Synth::writeGlobalRegister(uint8_t parameter, uint16_t value)
{
    const uint16_t registerNumber = (0b10 << 14) | (parameter << 8);
    writeRegister(registerNumber, value);
}
