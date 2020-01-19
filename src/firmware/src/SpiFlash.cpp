
#include "SpiFlash.hpp"

#include <stm32f4xx.h>

#include <printf.h>


namespace octane
{


SpiFlash::SpiFlash(Spi& rSpi) :
    m_rSpi(rSpi),
    m_ReadInProgress {false},
    m_WriteInProgress {false}
{
    // TODO
    setWriteProtect(false);
    setHold(false);
    setChipEnable(false);
}



void SpiFlash::setWriteProtect(bool value)
{
    // The output is active LOW
    auto regValue = value ? GPIO_BSRR_BR4 : GPIO_BSRR_BS4;
    GPIOC->BSRR = regValue;
}

void SpiFlash::setHold(bool value)
{
    // The output is active LOW
    auto regValue = value ? GPIO_BSRR_BR5 : GPIO_BSRR_BS5;
    GPIOC->BSRR = regValue;
}

void SpiFlash::setChipEnable(bool value)
{
    // The output is active LOW
    auto regValue = value ? GPIO_BSRR_BR4 : GPIO_BSRR_BS4;
    GPIOA->BSRR = regValue;
}



void SpiFlash::readBytes(std::uint8_t* pBuffer, std::uint32_t address, std::size_t count)
{
    m_ReadInProgress = true;

    uint8_t commandBytes[] = {
        0x03,  // READ
        static_cast<uint8_t>(address >> 16),
        static_cast<uint8_t>(address >> 8),
        static_cast<uint8_t>(address >> 0),
    };
    setChipEnable(true);
    printf("starting write \r\n");
    m_rSpi.writeBytes(commandBytes, sizeof(commandBytes));
    printf("waiting \r\n");
    while (m_rSpi.writeInProgress());  // wait
    printf("starting read \r\n");
    m_rSpi.readBytes(pBuffer, count);
    printf("waiting 2 \r\n");
    while (m_rSpi.readInProgress());  // wait  (TODO: remove this wait)
    printf("over \r\n");
    setChipEnable(false);
}


void SpiFlash::writeBytes(std::uint8_t* pBuffer, std::uint32_t address, std::size_t count)
{
    // Prior to execut-ing any Byte-Program,
    // Auto Address Increment (AAI) programming, Sector-Erase, Block-Erase,
    // Write-Status-Register,  or  Chip-Erase  instructions,  the  Write-Enable
    // (WREN)  instruction  must  be  executed first.

    m_WriteInProgress = true;

    // TODO

}


// bool SpiFlash::readInProgress() const
// {
//     if (m_ReadInProgress)
//     {
//         m_ReadInProgress = m_rSpi.readInProgress();
//     }
//     return m_ReadInProgress;
// }

// bool SpiFlash::writeInProgress() const
// {
//     if (m_WriteInProgress)
//     {
//         m_WriteInProgress = m_rSpi.writeInProgress();
//     }
//     return m_WriteInProgress;
// }




}
