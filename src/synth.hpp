
#ifndef SYNTH_HPP
#define SYNTH_HPP

#include <stddef.h>  // for size_t
#include <stdint.h>
#include <deque>
#include <queue>

#include "Vsynth.h"


class Synth
{
public:

    Synth();
    ~Synth();

    Synth(const Synth&) = delete;
    Synth& operator=(const Synth&) = delete;

    void tick();
    void spiTick();
    void reset();
    void writeRegister(uint16_t registerNumber, uint16_t registerValue);

    void spiSendReceive();

    void setNoteOn(uint8_t voiceNum, bool noteOn);
    bool getNoteOn(uint8_t voiceNum) const;

    void writeOperatorRegister(uint8_t voiceNum, uint8_t operatorNum, uint8_t parameter, uint16_t value);
    void populateSineTable();

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

    static const uint8_t OP_PARAM_PHASE_STEP  { 0x00 };
    static const uint8_t OP_PARAM_ALGORITHM      { 0x01 };

    static const uint8_t OP_PARAM_ENVELOPE_ATTACK_LEVEL   { 0x02 };
    static const uint8_t OP_PARAM_ENVELOPE_SUSTAIN_LEVEL  { 0x03 };
    static const uint8_t OP_PARAM_ENVELOPE_ATTACK_RATE    { 0x04 };
    static const uint8_t OP_PARAM_ENVELOPE_DECAY_RATE     { 0x05 };
    static const uint8_t OP_PARAM_ENVELOPE_RELEASE_RATE   { 0x06 };

    static const uint8_t PARAM_NOTEON_BANK0  { 0x10 };
    static const uint8_t PARAM_NOTEON_BANK1  { 0x11 };


private:
    Vsynth m_Synth;
    std::deque<int16_t> m_SampleBuffer;

    size_t m_SampleCounter;
    FILE* m_DataFile;

    bool m_NoteOnState[32];

    std::queue<uint16_t> m_SPI_SendQueue;
    uint16_t m_SPI_TickCounter;
    uint16_t m_SPI_OutputBuffer;
    uint16_t m_SPI_InputBuffer;

};


#endif
