
#include <array>


// TODO: remove
#include <printf.h>


#include "fpga.hpp"
#include "CriticalSection.hpp"




// TODO: Remove this dependency by wrapping in BSP functions
#include <stm32f4xx.h>


namespace octane
{

Fpga Fpga::instance;


Fpga::Fpga() :
    m_LatestSample {0xcccc},
    m_SpiTransferInProgress {false},
    m_CommandTransferProgress {0},
    m_NumCommands {0},
    m_CommandBuffer {},
    m_NoteOnState {false, false, false, false, false, false, false, false,  // TODO: better way
                   false, false, false, false, false, false, false, false,
                   false, false, false, false, false, false, false, false,
                   false, false, false, false, false, false, false, false}
{
    // Set NSS high until a write is requested
    GPIOB->BSRR = GPIO_BSRR_BS12;
}



void Fpga::clearNotesOn()
{
    for (auto& rState : m_NoteOnState)
    {
        rState = false;
    }

    writeOperatorRegister(0, 0, PARAM_NOTEON_BANK0, 0x0000);
    writeOperatorRegister(0, 0, PARAM_NOTEON_BANK1, 0x0000);
}

void Fpga::setNoteOn(uint8_t voiceNum, bool noteOn)
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





Fpga& Fpga::getInstance()
{
    return Fpga::instance;
}


void Fpga::reset()
{
    // TODO
}


void Fpga::onSpiRxComplete()
{
    m_LatestSample = SPI2->DR;
}


void Fpga::onSpiTxComplete()
{
    m_CommandTransferProgress += 1;

    if (m_CommandTransferProgress == m_NumCommands * HALF_WORDS_PER_COMMAND)
    {
        // Wait for the final transfer to complete.
        // TXE will be deasserted a few SPI cycles before the data is
        // actually finished transmitting to the slave.
        while (SPI2->SR & SPI_SR_BSY);
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

    SPI2->CR2 &= ~SPI_CR2_TXEIE;  // disable interrupts
    GPIOB->BSRR = GPIO_BSRR_BS12;  // release NSS when transfer completes
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

        GPIOB->BSRR = GPIO_BSRR_BR12;  // pull NSS low during DMA transfer
        SPI2->DR = m_CommandBuffer[m_CommandTransferProgress];  // begin transfer
        SPI2->CR2 |= SPI_CR2_TXEIE | SPI_CR2_RXNEIE;  // enable interrupts

        // printf("commitRegisterWrites \r\n");
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
