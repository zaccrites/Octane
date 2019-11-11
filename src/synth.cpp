
#include "synth.hpp"

#include <algorithm>  // for std::fill_n


Synth::Synth() :
    m_Synth {},
    m_SampleBuffer {},
    m_SampleCounter {0},
    m_DataFile { fopen("data.csv", "w") },
    m_NoteOnState {},
    m_SPI_SendQueue {},
    m_SPI_TickCounter {0}
{
    std::fill_n(m_NoteOnState, 32, false);

    m_Synth.i_SPI_NSS = 1;

}

Synth::~Synth()
{
    fclose(m_DataFile);
}


// void Synth::spiTick()
// {

//     m_Synth.i_SPI_NSS = 1;
//     m_Synth.i_SPI_SCK = 1;
//     tick();

//     m_Synth.i_SPI_NSS = 0;


//     // The FPGA clock runs 4x as fast as the SPI clock
//     // in order to avoid missing SCK edges.

//     m_Synth.i_SPI_SCK = 0;
//     tick();

//     // Output the MSB first
//     m_Synth.i_SPI_MOSI = (m_SPI_OutputBuffer & 0x8000) ? 1 : 0;
//     m_SPI_OutputBuffer = m_SPI_OutputBuffer << 1;
//     tick();

//     m_Synth.i_SPI_SCK = 1;
//     tick();

//     // Input the MSB first
//     m_SPI_InputBuffer = (m_SPI_InputBuffer << 1) | (m_Synth.o_SPI_MISO != 0);
//     tick();

//     const bool collectNewSample = ++m_SPI_TickCounter >= 256 / 4;
//     if (collectNewSample)
//     {
//         m_SPI_TickCounter = 0;

//         auto sample = static_cast<int16_t>(m_SPI_InputBuffer);

//         m_SampleBuffer.push_front(sample);
//         fprintf(m_DataFile, "%zu,%d\n", m_SampleCounter++, sample);
//     }
// }


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
        // TODO: This isn't really a "voice op" parameter anymore.
        // Need to reorganize registers now that I need all 14 bits
        // of a "global" address for the sine table.
        // If I can write the whole thing at once then this problem
        // probably goes away (could just have 8 bit addresses, honestly).
        writeOperatorRegister(0, 0, PARAM_NOTEON_BANK0, newRegisterValue);
    }
    else
    {
        writeOperatorRegister(0, 0, PARAM_NOTEON_BANK1, newRegisterValue);
    }
}


bool Synth::getNoteOn(uint8_t voiceNum) const
{
    return m_NoteOnState[voiceNum % 32];
}


void Synth::spiSendReceive()
{

    auto getCommandHalfWord = [this]() -> uint16_t {
        uint16_t value;
        if (m_SPI_SendQueue.empty())
        {
            value = 0;
        }
        else
        {
            value = m_SPI_SendQueue.front();
            m_SPI_SendQueue.pop();
        }
        return value;
    };

    const uint16_t spiRegWriteAddr = getCommandHalfWord();
    const uint16_t spiRegWriteValue = getCommandHalfWord();
    uint32_t spiOutputBuffer = (spiRegWriteAddr << 16) | spiRegWriteValue;
    uint32_t spiInputBuffer = 0;

    m_Synth.i_SPI_NSS = 0;
    tick();

    for (int i = 0; i < 32; i++)
    {
        // Rising edge of SCK isn't used
        m_Synth.i_SPI_SCK = 1;
        tick();

        // Output data (MSB out first)
        m_Synth.i_SPI_MOSI = (spiOutputBuffer & 0x80000000) ? 1 : 0;
        spiOutputBuffer = spiOutputBuffer << 1;
        tick();

        // Data is latched on the falling edge of SCK
        m_Synth.i_SPI_SCK = 0;
        tick();

        // Input the valid slave data (MSB in first)
        spiInputBuffer = (spiInputBuffer << 1) | m_Synth.o_SPI_MISO;
        tick();
    }

    m_Synth.i_SPI_NSS = 1;
    tick();

    const bool collectNewSample = ++m_SPI_TickCounter >= 8 / 4;
    if (collectNewSample)
    {
        m_SPI_TickCounter = 0;

        // TODO: REMOVE HACK
        // spiInputBuffer = spiInputBuffer << 1;

        auto sample = static_cast<int16_t>(spiInputBuffer & 0x0000ffff);
        if (false) printf(
            "Collected sample %d (0x%04x) (0b%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c)\n",
            sample,
            spiInputBuffer,

            ((spiInputBuffer & 0x8000) ? '1' : '0'),
            ((spiInputBuffer & 0x4000) ? '1' : '0'),
            ((spiInputBuffer & 0x2000) ? '1' : '0'),
            ((spiInputBuffer & 0x1000) ? '1' : '0'),
            ((spiInputBuffer & 0x0800) ? '1' : '0'),
            ((spiInputBuffer & 0x0400) ? '1' : '0'),
            ((spiInputBuffer & 0x0200) ? '1' : '0'),
            ((spiInputBuffer & 0x0100) ? '1' : '0'),
            ((spiInputBuffer & 0x0080) ? '1' : '0'),
            ((spiInputBuffer & 0x0040) ? '1' : '0'),
            ((spiInputBuffer & 0x0020) ? '1' : '0'),
            ((spiInputBuffer & 0x0010) ? '1' : '0'),
            ((spiInputBuffer & 0x0008) ? '1' : '0'),
            ((spiInputBuffer & 0x0004) ? '1' : '0'),
            ((spiInputBuffer & 0x0002) ? '1' : '0'),
            ((spiInputBuffer & 0x0001) ? '1' : '0')
        );

        m_SampleBuffer.push_front(sample);
        fprintf(m_DataFile, "%zu,%d\n", m_SampleCounter++, sample);
    }

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
