
#include "Spi.hpp"
#include "isr.hpp"

#include <printf.h>


namespace octane
{

// TODO: Add support for real DMA
//   This setup is essentially replicating the DMA-to-SPI engine
//   using the CPU to read and write to memory and interact
//   with the SPI peripheral.
//
//   For small transfers, this is fine. For larger ones
//   being able to use the real DMA engine will be more
//   efficient.
//


// Assumes that the GPIO is already set up
// TODO: Allow for 16 bit version too. (could use a template argument for uint8_t vs uint16_t)
Spi::Spi(SPI_TypeDef* pRawSpi) :
    m_pSpi {pRawSpi},
    m_IsBusy {false},
    m_CurrentCommand {nullptr, 0, nullptr, 0, 0}
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


bool Spi::isBusy() const
{
    return m_IsBusy;
}


void Spi::execute(const Spi::Command& rCommand)
{
    m_IsBusy = true;
    m_CurrentCommand = rCommand;

    m_pSpi->CR2 |= SPI_CR2_TXEIE | SPI_CR2_RXNEIE;
    transmitNextByte();
}


void Spi::transmitNextByte()
{
    if (m_CurrentCommand.m_NumTransmitWords > 0)
    {
        m_pSpi->DR = *m_CurrentCommand.m_pTransmitBuffer++;
        m_CurrentCommand.m_NumTransmitWords -= 1;
    }
    else
    {
        m_pSpi->DR = 0;
    }

    // The transfer is complete
    if (m_CurrentCommand.m_NumReceiveWords == 0 && m_CurrentCommand.m_NumTransmitWords == 0)
    {
        while (m_pSpi->SR & SPI_SR_BSY);
        m_pSpi->CR2 &= ~(SPI_CR2_TXEIE | SPI_CR2_RXNEIE);
        m_IsBusy = false;

        // Throw out any lingering dummy bytes
        (void)m_pSpi->DR;
    }
}


void Spi::onRxComplete()
{
    if (m_CurrentCommand.m_NumIgnoreReceiveWords > 0)
    {
        // Intentionally ignore this word because we're
        // not ready to start writing to the buffer.
        m_CurrentCommand.m_NumIgnoreReceiveWords -= 1;

        // Clear RXNE flag
        (void)m_pSpi->DR;
    }
    else if (m_CurrentCommand.m_NumReceiveWords == 0)
    {
        // Ignore this word because we're done receiving data.

        // Clear RXNE flag
        (void)m_pSpi->DR;
    }
    else
    {
        *m_CurrentCommand.m_pReceiveBuffer++ = m_pSpi->DR;
        m_CurrentCommand.m_NumReceiveWords -= 1;
    }
}


void Spi::onTxComplete()
{
    transmitNextByte();
}


}
