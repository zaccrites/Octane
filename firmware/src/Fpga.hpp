
#ifndef FPGA_HPP
#define FPGA_HPP

#include <stdint.h>



extern "C" void SPI2_IRQHandler();



namespace octane
{

class Fpga
{
public:

    static Fpga& getInstance();

    void reset();

    uint16_t getLatestSample() const { return m_LatestSample; }
    bool getSpiTransferInProgress() const { return m_SpiTransferInProgress; }

    void commitRegisterWrites();
    bool writeRegister(uint16_t registerNumber, uint16_t value);
    bool writeOperatorRegister(uint8_t voiceNum, uint8_t operatorNum, uint8_t parameter, uint16_t value);


    void clearNotesOn();
    void setNoteOn(uint8_t voiceNum, bool noteOn);
    bool getNoteOn(uint8_t voiceNum) const
    {
        return m_NoteOnState[voiceNum % 32];
    }


    static const size_t COMMAND_BUFFER_CAPACITY = 1024;
    static const size_t HALF_WORDS_PER_COMMAND = 2;


public:
    static const uint8_t OP_PARAM_PHASE_STEP  { 0x00 };
    static const uint8_t OP_PARAM_ALGORITHM      { 0x01 };

    static const uint8_t OP_PARAM_ENVELOPE_ATTACK_LEVEL   { 0x02 };
    static const uint8_t OP_PARAM_ENVELOPE_SUSTAIN_LEVEL  { 0x03 };
    static const uint8_t OP_PARAM_ENVELOPE_ATTACK_RATE    { 0x04 };
    static const uint8_t OP_PARAM_ENVELOPE_DECAY_RATE     { 0x05 };
    static const uint8_t OP_PARAM_ENVELOPE_RELEASE_RATE   { 0x06 };

    static const uint8_t OP_PARAM_FEEDBACK_LEVEL   { 0x07 };


    static const uint8_t PARAM_NOTEON_BANK0  { 0x10 };
    static const uint8_t PARAM_NOTEON_BANK1  { 0x11 };
    static const uint8_t PARAM_LED_CONFIG  { 0x12 };


private:
    friend void ::SPI2_IRQHandler();

    Fpga();
    static Fpga instance;

    // bool getNextCommand(FpgaCommand rCommand);

    void onSpiRxComplete();
    void onSpiTxComplete();
    void onSpiTransferComplete();

private:

    uint16_t m_LatestSample;

    bool m_SpiTransferInProgress;

    size_t m_CommandTransferProgress;
    size_t m_NumCommands;
    uint16_t m_CommandBuffer[COMMAND_BUFFER_CAPACITY * HALF_WORDS_PER_COMMAND];


    bool m_NoteOnState[32];


    // size_t m_CommandBufferStart;
    // size_t m_CommandBufferCount;
    // FpgaCommand m_CommandBuffer[COMMAND_BUFFER_CAPACITY];

    // bool m_HasCurrentCommand;
    // FpgaCommand m_CurrentCommand;


    // TODO: Better way to start the first transaction?
    // Also relevant if disabling SPI between comms
    // bool m_SpiStarted;

};

}

#endif
