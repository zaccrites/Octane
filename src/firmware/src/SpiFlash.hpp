
#ifndef SPI_FLASH_HPP
#define SPI_FLASH_HPP


#include <cstdint>

#include "Spi.hpp"



namespace octane
{

class SpiFlash
{
public:

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

    // bool readInProgress() const;
    // bool writeInProgres() const;


private:

    Spi& m_rSpi;

    mutable bool m_ReadInProgress;
    mutable bool m_WriteInProgress;

};

}


#endif
