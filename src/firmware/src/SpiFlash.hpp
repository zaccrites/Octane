
#ifndef SPI_FLASH_HPP
#define SPI_FLASH_HPP


#include <cstdint>

#include "Spi.hpp"



namespace octane
{


//
// Compatible with the SST25VF020B SPI Flash
//
class SpiFlash
{
public:

    static const std::uint8_t SR_BUSY { 0x01 };
    static const std::uint8_t SR_WEL { 0x02 };
    static const std::uint8_t SR_BP0 { 0x04 };
    static const std::uint8_t SR_BP1 { 0x08 };
    // static const std::uint8_t <reserved> { 0x10 };
    // static const std::uint8_t <reserved> { 0x20 };
    static const std::uint8_t SR_AAI { 0x40 };
    static const std::uint8_t SR_BPL { 0x80 };

    // static const std::uint8_t <reserved> { 0x01 };
    // static const std::uint8_t <reserved> { 0x02 };
    static const std::uint8_t SR1_TSP { 0x04 };
    static const std::uint8_t SR1_BSP { 0x08 };
    // static const std::uint8_t <reserved> { 0x10 };
    // static const std::uint8_t <reserved> { 0x20 };
    // static const std::uint8_t <reserved> { 0x40 };
    // static const std::uint8_t <reserved> { 0x80 };




    // Assumes GPIO is already set up
    SpiFlash(Spi& rRawSpi);

    void setWriteProtect(bool value);
    void setHold(bool value);
    void setChipEnable(bool value);

    void readBytes(std::uint8_t* pBuffer, std::uint32_t address, std::size_t count);
    void writeBytes(std::uint8_t* pBuffer, std::uint32_t address, std::size_t count);

    void eraseChip();

    void setWriteEnable(bool value);

    void jedecReadId();

    std::uint8_t readStatusRegister();
    std::uint8_t readSoftwareStatusRegister();

    // bool readInProgress() const;
    // bool writeInProgres() const;




private:

    Spi& m_rSpi;

    mutable bool m_ReadInProgress;
    mutable bool m_WriteInProgress;

};


}


#endif
