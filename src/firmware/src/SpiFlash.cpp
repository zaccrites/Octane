
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

    const uint8_t READ_COMMAND { 0x03 };
    const uint8_t command[] = {
        READ_COMMAND,
        static_cast<uint8_t>(address >> 16),
        static_cast<uint8_t>(address >> 8),
        static_cast<uint8_t>(address >> 0),
    };

    setChipEnable(true);
    m_rSpi.execute({command, sizeof(command), pBuffer, count, sizeof(command)});
    while (m_rSpi.isBusy());
    setChipEnable(false);
}


void SpiFlash::writeBytes(std::uint8_t* pBuffer, std::uint32_t address, std::size_t count)
{
    // Prior to execut-ing any Byte-Program,
    // Auto Address Increment (AAI) programming, Sector-Erase, Block-Erase,
    // Write-Status-Register,  or  Chip-Erase  instructions,  the  Write-Enable
    // (WREN)  instruction  must  be  executed first.


    // Also, the byte must be erased before it can be written.


    m_WriteInProgress = true;


    // TODO: Use AAI Word Program instead
    // const uint8_t AAI_WORD_PROGRAM_COMMAND { 0xad };

    const uint8_t BYTE_PROGRAM_COMMAND { 0x02 };

    for (std::size_t i = 0; i < count; i++)
    {
        const uint8_t command[] = {
            BYTE_PROGRAM_COMMAND,
            static_cast<uint8_t>((address + i) >> 16),
            static_cast<uint8_t>((address + i) >> 8),
            static_cast<uint8_t>((address + i) >> 0),
            pBuffer[i],
        };
        setChipEnable(true);
        m_rSpi.execute({command, sizeof(command), nullptr, 0, 0});
        while (m_rSpi.isBusy());
        setChipEnable(false);
        while ((readStatusRegister() & SR_BUSY) != 0);
    }

}



std::uint8_t SpiFlash::readStatusRegister()
{
    const uint8_t READ_STATUS_REGISTER_COMMAND { 0x05 };
    uint8_t response;
    setChipEnable(true);
    m_rSpi.execute({&READ_STATUS_REGISTER_COMMAND, 1, &response, 1, 1});
    while (m_rSpi.isBusy());
    setChipEnable(false);
    return response;
}


std::uint8_t SpiFlash::readSoftwareStatusRegister()
{
    const uint8_t READ_SOFTWARE_STATUS_REGISTER { 0x35 };
    uint8_t response;
    setChipEnable(true);
    m_rSpi.execute({&READ_SOFTWARE_STATUS_REGISTER, 1, &response, 1, 1});
    while (m_rSpi.isBusy());
    setChipEnable(false);
    return response;
}




void SpiFlash::eraseChip()
{
    const uint8_t CHIP_ERASE_COMMAND { 0x60 };  // or 0xc7
    const uint8_t command[] = {CHIP_ERASE_COMMAND};

    setChipEnable(true);
    m_rSpi.execute({command, sizeof(command), nullptr, 0, 0});
    while (m_rSpi.isBusy());
    setChipEnable(false);
    while ((readStatusRegister() & SR_BUSY) != 0);  // wait for erase to complete

    // T_CE = 50 ms
}


void SpiFlash::setWriteEnable(bool value)
{
    const uint8_t WRITE_ENABLE_COMMAND { 0x06 };
    const uint8_t WRITE_DISABLE_COMMAND { 0x04 };
    const uint8_t command[] = {
        value ? WRITE_ENABLE_COMMAND : WRITE_DISABLE_COMMAND
    };

    setChipEnable(true);
    m_rSpi.execute({command, sizeof(command), nullptr, 0, 0});
    while (m_rSpi.isBusy());
    setChipEnable(false);

    // TODO: poll status register until operation completes?
}


void SpiFlash::jedecReadId()
{
    const uint8_t JEDEC_READ_ID_COMMAND { 0x9f };
    const uint8_t command[] = { JEDEC_READ_ID_COMMAND };
    uint8_t response[3];

    setChipEnable(true);
    m_rSpi.execute({command, sizeof(command), response, sizeof(response), sizeof(command)});
    while (m_rSpi.isBusy());
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
