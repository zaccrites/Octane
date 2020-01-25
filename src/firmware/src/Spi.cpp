
#include "Spi.hpp"
#include "isr.hpp"

#include <printf.h>


namespace octane
{


// Assumes that the GPIO is already set up
// TODO: Allow for 16 bit version too. (could use a template argument for uint8_t vs uint16_t)
Spi::Spi(SPI_TypeDef* pRawSpi) :
    m_pSpi {pRawSpi},
    m_TxByteCount {0},
    m_RxByteCount {0},
    m_pTxSrcTarget {nullptr},
    m_pRxDestTarget {nullptr}
{
    m_pSpi->CR1 =
        SPI_CR1_MSTR |                     // act as SPI master
        (0b011 << SPI_CR1_BR_Pos) |        // bitrate
        0 /* SPI_CR1_LSBFIRST */ |         // send MSB first
        0 /* SPI_CR1_DFF */ |              // 8 bit frame
        0 /* SPI_CR1_CPHA */ |             // clock phase (rising clock edge captures data)
        0 /* SPI_CR1_CPOL */ |             // clock polarity (clock to zero when idle)
        SPI_CR1_SSI | SPI_CR1_SSM;         // software slave management

    octane::isr::registerSpiHandler(pRawSpi, this);

    // Enable the peripheral
    m_pSpi->CR1 |= SPI_CR1_SPE;
}

std::uint8_t Spi::readByte()
{
    // ???
    std::uint8_t result;
    readBytes(&result, 1);
    return result;
}

void Spi::readBytes(std::uint8_t* pBuffer, std::size_t count)
{
    m_pRxDestTarget = pBuffer;
    m_RxByteCount = count;
    m_pSpi->CR2 |= SPI_CR2_RXNEIE;

    // TODO: Need to write the next byte,
    // or write a dummy byte if there isn't
    // anything to send. Need to write
    // *something* to keep the clock going.
    m_pSpi->DR = 0;

}

void Spi::writeByte(std::uint8_t value)
{
    // ???
    writeBytes(&value, 1);
}

void Spi::writeBytes(const std::uint8_t* pBuffer, std::size_t count)
{
    m_pTxSrcTarget = pBuffer;
    m_TxByteCount = count;
    m_pSpi->CR2 |= SPI_CR2_TXEIE;

    // printf("Sending %x \r\n", *m_pTxSrcTarget);
    m_pSpi->DR = *m_pTxSrcTarget++;
}


bool Spi::readInProgress() const
{
    return m_RxByteCount > 0;
}

bool Spi::writeInProgress() const
{
    return m_TxByteCount > 0;
}


void Spi::onRxComplete()
{
    *m_pRxDestTarget++ = m_pSpi->DR;
    // printf("Got %02x \r\n", *(m_pRxDestTarget - 1));

    // TODO: Need to write the next byte,
    // or write a dummy byte if there isn't
    // anything to send. Need to write
    // *something* to keep the clock going.
    if ( ! writeInProgress())
    {
        m_pSpi->DR = 0;
    }


    m_RxByteCount -= 1;
    if (m_RxByteCount == 0)
    {

        while (m_pSpi->SR & SPI_SR_BSY);


        // The transfer is complete
        m_pSpi->CR2 &= ~SPI_CR2_RXNEIE;
    }
}

void Spi::onTxComplete()
{
    m_TxByteCount -= 1;
    if (m_TxByteCount > 0)
    {
        // printf("Sending %02x \r\n", *m_pTxSrcTarget);
        m_pSpi->DR = *m_pTxSrcTarget++;
    }
    else
    {
        // Wait for the final transfer to complete.
        // TXE will be deasserted a few SPI cycles before the data is
        // actually finished transmitting to the slave.
        while (m_pSpi->SR & SPI_SR_BSY);

        // The transfer is complete
        m_pSpi->CR2 &= ~SPI_CR2_TXEIE;
    }
}


}
