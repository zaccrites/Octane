
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

    struct Command
    {
        const std::uint8_t* m_pTransmitBuffer;
        std::uint32_t m_NumTransmitWords;

        std::uint8_t* m_pReceiveBuffer;
        std::uint32_t m_NumReceiveWords;
        std::uint32_t m_NumIgnoreReceiveWords;

        Command(const std::uint8_t* pTransmitBuffer, std::uint32_t numTransmitWords, std::uint8_t* pReceiveBuffer, std::uint32_t numReceiveWords, std::uint32_t numIgnoreReceiveWords) :
            m_pTransmitBuffer {pTransmitBuffer},
            m_NumTransmitWords {numTransmitWords},
            m_pReceiveBuffer {pReceiveBuffer},
            m_NumReceiveWords {numReceiveWords},
            m_NumIgnoreReceiveWords {numIgnoreReceiveWords}
        {
        }

        bool stillReceiving() const;
        bool stillTransmitting() const;

        std::uint8_t getNextTransmitWord();
        void handleReceivedWord(std::uint8_t word);


    };

    // Assumes that the GPIO is already set up
    // TODO: Allow for 16 bit version too (probably via template)
    Spi(SPI_TypeDef* pSpi);

    void execute(const Command& rCommand);

    bool isBusy() const;

    // TODO: Make these private and use a friend function for the caller?
    void onTransmitBufferEmpty();
    void onReceiveBufferNotEmpty();

private:

    SPI_TypeDef* m_pSpi;

    bool m_IsBusy;
    Command m_CurrentCommand;

};


}


#endif
