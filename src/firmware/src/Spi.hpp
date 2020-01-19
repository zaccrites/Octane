
#include "stm32f4xx.h"

#include <cstdint>


#ifndef SPI_HPP
#define SPI_HPP

namespace octane
{

// TODO: Allow reading and writing via DMA
class Spi
{
public:

    // Assumes that the GPIO is already set up
    // TODO: Allow for 16 bit version too
    Spi(SPI_TypeDef* pSpi);

    std::uint8_t readByte();
    void readBytes(std::uint8_t* pBuffer, std::size_t count);

    void writeByte(std::uint8_t value);
    void writeBytes(const std::uint8_t* pBuffer, std::size_t count);

    bool readInProgress() const;
    bool writeInProgress() const;

    // TODO: Make these private and use a friend function for the caller?
    void onTxComplete();
    void onRxComplete();

private:


    SPI_TypeDef* m_pSpi;

    std::size_t m_TxByteCount;
    std::size_t m_RxByteCount;
    const std::uint8_t* m_pTxSrcTarget;
    std::uint8_t* m_pRxDestTarget;

};

}



#endif
