
#include <array>

#include "fpga.hpp"

// TODO: Generate sine table at boot instead
#define SINE_TABLE_IMPLEMENTATION
#include "sine_table.h"


// TODO: Remove this dependency by wrapping in BSP functions
#include <stm32f4xx.h>



uint16_t lastSample;
// bool fpgaSpiReady = true;




namespace octane::fpga
{


// TODO: Move to "synth" file instead?


// uint16_t phaseStepForFrequency(double frequency) {
//     // Formula:
//     // phaseStep = 2^N * f / FS
//     // where N is the number of bits of phase accumulation
//     // FS is the sample frequency
//     // and f is the desired tone frequency
//     return static_cast<uint16_t>(
//         static_cast<double>(1 << 16) *
//         frequency /
//         static_cast<double>(SAMPLE_FREQUENCY)
//     );
// }


static bool m_NoteOnState[32];

void clearNotesOn()
{
    for (auto& state : m_NoteOnState) state = false;

    writeOperatorRegister(0, 0, PARAM_NOTEON_BANK0, 0x0000);
    writeOperatorRegister(0, 0, PARAM_NOTEON_BANK1, 0x0000);
}

void setNoteOn(uint8_t voiceNum, bool noteOn)
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





void reset()
{
    // TODO: This is a BSP thing

    GPIOB->BSRR = GPIO_BSRR_BS8;
    // wait?
    GPIOB->BSRR = GPIO_BSRR_BR8;
}





void init()
{
    // Populate sine table memory
    for (size_t i = 0; i < 16 * 1024; i++)
    {
        const uint16_t registerNumber = (0b11 << 14) | i;
        writeRegister(registerNumber, SINE_TABLE[i]);
    }

    std::array<uint16_t, 8> algorithmWords = {
        //       7654321
        //xxxxxx mmmmmmm nnn c
        0b000000'0000000'000'0,  // OP1
        0b000000'0000001'000'1,  // OP2
        0b000000'0000000'000'0,  // OP3
        0b000000'0000000'000'0,  // OP4
        0b000000'0000000'000'0,  // OP5
        0b000000'0000000'000'0,  // OP6
        0b000000'0000000'000'0,  // OP7
        0b000000'0000000'000'0,  // OP8
    };

    clearNotesOn();


    for (uint16_t voiceNum = 0; voiceNum < 32; voiceNum++)
    {
        for (uint16_t opNum = 0; opNum < 8; opNum++)
        {

            uint8_t feedbackLevel = (opNum == 0) ? 255 : 0;
            writeOperatorRegister(voiceNum, opNum, OP_PARAM_FEEDBACK_LEVEL, feedbackLevel);

            writeOperatorRegister(voiceNum, opNum, OP_PARAM_ALGORITHM, algorithmWords[opNum]);

            uint16_t phaseStep = 600;
            writeOperatorRegister(voiceNum, opNum, OP_PARAM_PHASE_STEP, phaseStep);

            // uint16_t attackLevel = 1000;
            // uint16_t sustainLevel = 1000;
            // uint16_t attackRate = 1000;
            // uint16_t decayRate = 1000;
            // uint16_t releaseRate = 1000;

            writeOperatorRegister(voiceNum, opNum, OP_PARAM_ENVELOPE_ATTACK_LEVEL, 0xffff); //fixOperatorLevel(attackLevel));
            writeOperatorRegister(voiceNum, opNum, OP_PARAM_ENVELOPE_SUSTAIN_LEVEL, 0xffff); //fixOperatorLevel(sustainLevel));
            writeOperatorRegister(voiceNum, opNum, OP_PARAM_ENVELOPE_ATTACK_RATE, 0xffff); // fixOperatorRate(attackRate));
            writeOperatorRegister(voiceNum, opNum, OP_PARAM_ENVELOPE_DECAY_RATE, 0xffff); // fixOperatorRate(decayRate));
            writeOperatorRegister(voiceNum, opNum, OP_PARAM_ENVELOPE_RELEASE_RATE, 0xffff); // fixOperatorRate(releaseRate));
        }
    }


}


void writeRegister(uint16_t registerNumber, uint16_t value)
{
    // TODO
    // m_SPI_SendQueue.push(registerNumber);
    // m_SPI_SendQueue.push(value);

    while ( ! (SPI2->SR & SPI_SR_TXE));
    SPI2->DR = registerNumber;
    while ( ! (SPI2->SR & SPI_SR_TXE));
    SPI2->DR = value;

    // TODO: Remove this
    while ( ! (SPI2->SR & SPI_SR_RXNE));
    lastSample = SPI2->DR;

}


void writeOperatorRegister(uint8_t voiceNum, uint8_t operatorNum, uint8_t parameter, uint16_t value)
{
    voiceNum = voiceNum % 32;  // 5 bits
    operatorNum = operatorNum % 8;  // 3 bits
    parameter = parameter % 64;  // 6 bits

    const uint16_t registerNumber = (0b10 << 14) | (parameter << 8) | (operatorNum << 5) | voiceNum;
    writeRegister(registerNumber, value);
}


}
