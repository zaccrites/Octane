
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
    // return m_IsBusy;
    // printf("recv=%d, ign=%d, tx=%d \r\n", m_CurrentCommand.m_NumReceiveWords, m_CurrentCommand.m_NumIgnoreReceiveWords, m_CurrentCommand.m_NumTransmitWords);
    return m_CurrentCommand.stillReceiving() || m_CurrentCommand.stillTransmitting();
}


void Spi::execute(const Spi::Command& rCommand)
{
    m_IsBusy = true;
    m_CurrentCommand = rCommand;

    m_pSpi->CR2 |= SPI_CR2_TXEIE | SPI_CR2_RXNEIE;
    // The TXE interrupt fires immediately, since the transmit buffer is empty.
}


void Spi::onReceiveBufferNotEmpty()
{
    if (m_CurrentCommand.stillReceiving())
    {
        printf("recv = %d \r\n", m_CurrentCommand.m_NumReceiveWords);
        m_CurrentCommand.handleReceivedWord(m_pSpi->DR);
    }
    else
    {
        // Ignore this word because we're done receiving data.
        // m_pSpi->CR2 &= ~(SPI_CR2_RXNEIE | SPI_CR2_TXEIE);

        // while (m_pSpi->SR & SPI_SR_BSY);
        // m_IsBusy = false;

        // Clear RXNE flag
        (void)m_pSpi->DR;
    }
}


void Spi::onTransmitBufferEmpty()
{
    // TODO: The TXE interrupt happens first, so if the last byte is incoming
    // but hasn't been processed yet, we'll send out a dummy byte.
    // Somehow we need to prevent doing that. Is this the right way?
    if ((m_CurrentCommand.m_NumReceiveWords + m_CurrentCommand.m_NumIgnoreReceiveWords) <= 1)
    {
        // Don't send another byte-- we just have to process the last one,
        // which is currently incoming.
        return;
    }

    if (m_CurrentCommand.stillTransmitting() || m_CurrentCommand.stillReceiving())
    {
        printf("tx = %d \r\n", m_CurrentCommand.m_NumTransmitWords);
        m_pSpi->DR = m_CurrentCommand.getNextTransmitWord();
    }
}


bool Spi::Command::stillReceiving() const
{
    return m_NumReceiveWords + m_NumIgnoreReceiveWords > 0;
}


bool Spi::Command::stillTransmitting() const
{
    return m_NumTransmitWords > 0;
}

std::uint8_t Spi::Command::getNextTransmitWord()
{
    if (m_NumTransmitWords > 0)
    {
        m_NumTransmitWords -= 1;
        return *m_pTransmitBuffer++;
    }

    const std::uint8_t DUMMY_BYTE { 0xff };
    return DUMMY_BYTE;
}

void Spi::Command::handleReceivedWord(std::uint8_t word)
{
    if (m_NumIgnoreReceiveWords > 0)
    {
        m_NumIgnoreReceiveWords -= 1;
    }
    else if (m_NumReceiveWords > 0)
    {
        m_NumReceiveWords -= 1;
        *m_pReceiveBuffer++ = word;
    }
    else
    {
        // Do nothing
    }
}


}
