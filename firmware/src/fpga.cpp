
#include <array>


// TODO: remove
#include <printf.h>


#include "fpga.hpp"
#include "CriticalSection.hpp"

// TODO: Generate sine table at boot instead
#define SINE_TABLE_IMPLEMENTATION
#include "sine_table.h"


// TODO: Remove this dependency by wrapping in BSP functions
#include <stm32f4xx.h>



uint16_t lastSample;
// bool fpgaSpiReady = true;


namespace octane
{

static Fpga f_FpgaInstance;


Fpga::Fpga() :
    m_LatestSample {0xcccc},
    m_SpiTransferInProgress {false},
    m_CommandTransferProgress {0},
    m_NumCommands {0},
    m_CommandBuffer {}
{
    // Set NSS high until a write is requested
    GPIOB->BSRR = GPIO_BSRR_BS12;
}


Fpga& Fpga::getInstance()
{
    return f_FpgaInstance;
}


void Fpga::reset()
{
}


void Fpga::onSpiRxComplete()
{
    m_LatestSample = SPI2->DR;
}


void Fpga::onSpiTxComplete()
{
    // if ( ! m_HasCurrentCommand)
    // {
    //     GPIOD->BSRR = GPIO_BSRR_BS12;


    //     GPIOB->BSRR = GPIO_BSRR_BR12;
    //     m_HasCurrentCommand = true;
    //     m_CurrentCommand = getNextCommand();
    //     SPI2->DR = m_CurrentCommand.registerNumber;d
    // }
    // else
    // {

    //     SPI2->DR = m_CurrentCommand.value;
    //     m_HasCurrentCommand = false;

    //     // TODO: only release this after the second half finishes,
    //     // and only if there isn't another command to send
    //     GPIOB->BSRR = GPIO_BSRR_BS12;
    // }

    // SPI2->CR2 |= SPI_CR2_TXEIE;  // do I need to only set this AFTER setting DR?

    m_CommandTransferProgress += 1;

    if (m_CommandTransferProgress == m_NumCommands * HALF_WORDS_PER_COMMAND)
    {
        onSpiTransferComplete();
    }
    else
    {
        // Send the next 16 bits
        SPI2->DR = m_CommandBuffer[m_CommandTransferProgress];
    }

}


void Fpga::onSpiTransferComplete()
{
    // Disable SPI? Must wait until TXE=1 and then BSY=0

    SPI2->CR2 &= ~(SPI_CR2_TXEIE | SPI_CR2_RXNEIE);  // disable interrupts
    GPIOD->BSRR = GPIO_BSRR_BS12;  // release NSS when transfer completes
    m_SpiTransferInProgress = false;
    m_NumCommands = 0;
}


void Fpga::commitRegisterWrites()
{
    if (m_NumCommands > 0)
    {
        m_CommandTransferProgress = 0;
        m_SpiTransferInProgress = true;

        // Enable SPI?

        GPIOD->BSRR = GPIO_BSRR_BR12;  // pull NSS low during DMA transfer
        SPI2->DR = m_CommandBuffer[m_CommandTransferProgress];  // begin transfer
        SPI2->CR2 |= SPI_CR2_TXEIE | SPI_CR2_RXNEIE;  // enable interrupts
    }
}



// TODO: Get sound working with plain interrupts before worrying about DMA

// Can use the same interface. On commit, enable TXEIE interrupt and begin writing
// to DR. If there are still items, then write to DR again. Otherwise disable
// interrupts and call onDmaComplete (though this is a misnomer, perhaps rename
// it to onTransferComplete or something). Pull down NSS when you start and
// release it when the transfer is finished.

// DMA will be useful later, but since the SPI interrupts are working now
// I'd like to try using them first. Once I get some sound output actually
// working and I need those cycles to do other work, I can revisit that.






bool Fpga::writeRegister(uint16_t registerNumber, uint16_t value)
{
    CriticalSectionLock lock;

    if (m_NumCommands < COMMAND_BUFFER_CAPACITY && ! m_SpiTransferInProgress)
    {
        m_CommandBuffer[2 * m_NumCommands + 0] = registerNumber;
        m_CommandBuffer[2 * m_NumCommands + 1] = value;

        m_NumCommands += 1;
    }

    // if (m_CommandBufferCount < COMMAND_BUFFER_CAPACITY)
    // {
    //     const size_t index = (m_CommandBufferStart + m_CommandBufferCount++) % COMMAND_BUFFER_CAPACITY;
    //     m_CommandBuffer[index] = {registerNumber, value};

    //     if ( ! m_SpiStarted)
    //     {
    //         SPI2->DR = 0x0000;
    //         SPI2->CR2 |= SPI_CR2_TXEIE;  // do I need to only set this AFTER setting DR?

    //         // dummy write to kick it off?
    //         // Should be ignored since NSS is high.
    //         // TODO: Find a better way once DMA is implemented
    //         m_SpiStarted = true;
    //     }
    // }
}



bool Fpga::writeOperatorRegister(uint8_t voiceNum, uint8_t operatorNum, uint8_t parameter, uint16_t value)
{
    voiceNum = voiceNum % 32;  // 5 bits
    operatorNum = operatorNum % 8;  // 3 bits
    parameter = parameter % 64;  // 6 bits

    const uint16_t registerNumber = (0b10 << 14) | (parameter << 8) | (operatorNum << 5) | voiceNum;
    return writeRegister(registerNumber, value);
}



}





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

    // Set NSS high until a write is requested
    GPIOB->BSRR = GPIO_BSRR_BS12;


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



    // while ( ! (SPI2->SR & SPI_SR_TXE));
    // SPI2->DR = registerNumber;
    // while ( ! (SPI2->SR & SPI_SR_TXE));
    // SPI2->DR = value;

    // These need to be reversed for some reason... or do they? idk


    // SPI2->CR1 |= SPI_CR1_SPE;    // enable SPI before comms


    GPIOD->BSRR = GPIO_BSRR_BS12;  // LED

    GPIOB->BSRR = GPIO_BSRR_BR12;  // pull NSS low

    while (SPI2->SR & SPI_SR_TXE == 0);
    SPI2->DR = registerNumber;

    // while (SPI2->SR & SPI_SR_TXE == 0);
    // while (SPI2->SR & SPI_SR_BSY == 0);
    // while (SPI2->SR & SPI_SR_RXNE == 0);



    // GPIOB->BSRR = GPIO_BSRR_BS12;  // release NSS
    // SPI2->DR = value;

    // while ( ! (SPI2->SR & SPI_SR_TXE));
    // SPI2->DR = registerNumber;
    // while ( ! (SPI2->SR & SPI_SR_TXE));


    // while ( ! (SPI2->SR & SPI_SR_TXE));





    // GPIOD->BSRR = GPIO_BSRR_BR12;  // LED

    // SPI2->CR1 &= ~SPI_CR1_SPE;    // disable SPI to release NSS

    // TODO: Remove this
    // while ( ! (SPI2->SR & SPI_SR_RXNE));
    // lastSample = SPI2->DR;





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
