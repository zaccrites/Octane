
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

    const uint8_t CHIP_READ_COMMAND { 0x03 };
    const uint8_t commandBytes[] = {
        CHIP_READ_COMMAND,
        static_cast<uint8_t>(address >> 16),
        static_cast<uint8_t>(address >> 8),
        static_cast<uint8_t>(address >> 0),
    };
    setChipEnable(true);
    m_rSpi.writeBytes(commandBytes, sizeof(commandBytes));
    while (m_rSpi.writeInProgress());  // wait
    m_rSpi.readBytes(pBuffer, count);
    while (m_rSpi.readInProgress());  // wait  (TODO: remove this wait)
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

void SpiFlash::eraseChip()
{
    const uint8_t CHIP_ERASE_COMMAND { 0x60 };  // or 0xc7

    setChipEnable(true);
    m_rSpi.writeByte(CHIP_ERASE_COMMAND);
    setChipEnable(false);

    // T_CE = 50 ms
}


void SpiFlash::setWriteEnable(bool value)
{
    const uint8_t WRITE_ENABLE_COMMAND { 0x06 };
    const uint8_t WRITE_DISABLE_COMMAND { 0x04 };

    setChipEnable(true);
    m_rSpi.writeByte(value ? WRITE_ENABLE_COMMAND : WRITE_DISABLE_COMMAND);
    setChipEnable(false);

    // TODO: poll status register until operation completes?
}


void SpiFlash::jedecReadId()
{
    uint8_t response[3];

    setChipEnable(true);
    m_rSpi.writeByte(0x9f);
    while(m_rSpi.writeInProgress());
    m_rSpi.readBytes(response, sizeof(response));
    while(m_rSpi.readInProgress());
    setChipEnable(false);

    printf("JEDEC Manufacturer ID: %02Xh \r\n", response[0]);
    printf("          Memory Type: %02Xh \r\n", response[1]);
    printf("      Memory Capacity: %02Xh \r\n", response[2]);
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
