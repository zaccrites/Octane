
#include <cstdint>

#include <stm32f4xx.h>
#include <printf.h>

#include "Spi.hpp"


namespace octane::isr
{


// Count millisecond ticks
volatile std::uint32_t ticks = 0;
extern "C" void SysTick_Handler()
{
    ticks += 1;
}


extern "C" void Real_HardFault_Handler(std::uint32_t* registers)
{

    (void)registers;

    printf("HardFault_Handler \r\n");
    printf("  SCB->HFSR = 0x%08x \r\n", SCB->HFSR);

    // http://blog.feabhas.com/2013/02/developing-a-generic-hard-fault-handler-for-arm-cortex-m3cortex-m4/
    if (SCB->HFSR & SCB_HFSR_FORCED_Msk)
    {
        printf("  SCB->CFSR = 0x%08x \r\n", SCB->CFSR);

        if (SCB->CFSR & SCB_CFSR_BFARVALID_Msk) printf("BFARVALID \r\n");
        if (SCB->CFSR & SCB_CFSR_LSPERR_Msk) printf("LSPERR \r\n");
        if (SCB->CFSR & SCB_CFSR_STKERR_Msk) printf("STKERR \r\n");
        if (SCB->CFSR & SCB_CFSR_UNSTKERR_Msk) printf("UNSTKERR \r\n");
        if (SCB->CFSR & SCB_CFSR_IMPRECISERR_Msk) printf("IMPRECISERR \r\n");
        if (SCB->CFSR & SCB_CFSR_PRECISERR_Msk) printf("PRECISERR \r\n");
        if (SCB->CFSR & SCB_CFSR_IBUSERR_Msk) printf("IBUSERR \r\n");

        if (SCB->CFSR & SCB_CFSR_DIVBYZERO_Msk) printf("DIVBYZERO \r\n");
        if (SCB->CFSR & SCB_CFSR_UNALIGNED_Msk) printf("UNALIGNED \r\n");
        if (SCB->CFSR & SCB_CFSR_NOCP_Msk) printf("NOCP \r\n");
        if (SCB->CFSR & SCB_CFSR_INVPC_Msk) printf("INVPC \r\n");
        if (SCB->CFSR & SCB_CFSR_INVSTATE_Msk) printf("INVSTATE \r\n");
        if (SCB->CFSR & SCB_CFSR_UNDEFINSTR_Msk) printf("UNDEFINSTR \r\n");

    }

    // The Cortex-M exception system provides a set of registers to
    // the handler function.
    printf("r0  = 0x%08x \r\n", registers[0]);
    printf("r1  = 0x%08x \r\n", registers[1]);
    printf("r2  = 0x%08x \r\n", registers[2]);
    printf("r3  = 0x%08x \r\n", registers[3]);
    printf("r12 = 0x%08x \r\n", registers[4]);
    printf("lr  = 0x%08x \r\n", registers[5]);
    printf("pc  = 0x%08x \r\n", registers[6]);
    printf("psr = 0x%08x \r\n", registers[7]);


    while(true);
}

extern "C" void MemManage_Handler()
{
    printf("MemManage_Handler \r\n");
    while(true);
}

extern "C" void BusFault_Handler()
{
    printf("BusFault_Handler \r\n");
    while(true);
}

extern "C" void UsageFault_Handler()
{
    printf("UsageFault_Handler \r\n");
    while(true);
}




static void handleSpiInterrupt(SPI_TypeDef* pRawSpi, Spi* pSpi)
{
    if (pSpi != nullptr)
    {
        // TODO: Better way?
        // Maybe just call the function and let it not do anything.
        // A single interrupt handler for TXE and RXNE?
        if ((pRawSpi->SR & SPI_SR_TXE) && (pRawSpi->CR2 & SPI_CR2_TXEIE))
        {
            pSpi->onTransmitBufferEmpty();
        }

        if ((pRawSpi->SR & SPI_SR_RXNE) && (pRawSpi->CR2 & SPI_CR2_RXNEIE))
        {
            pSpi->onReceiveBufferNotEmpty();
        }
    }
}

static Spi* pSpi1 = nullptr;
extern "C" void SPI1_IRQHandler()
{
    handleSpiInterrupt(SPI1, pSpi1);
}

static Spi* pSpi2 = nullptr;
extern "C" void SPI2_IRQHandler()
{
    handleSpiInterrupt(SPI2, pSpi2);
}

static Spi* pSpi5 = nullptr;
extern "C" void SPI5_IRQHandler()
{
    handleSpiInterrupt(SPI5, pSpi5);
}

void registerSpiHandler(SPI_TypeDef* pRawSpi, Spi* pSpi)
{
    if (pRawSpi == SPI1)
    {
        pSpi1 = pSpi;
    }
    else if (pRawSpi == SPI2)
    {
        pSpi2 = pSpi;
    }
    else if (pRawSpi == SPI5)
    {
        pSpi5 = pSpi;
    }
}


}
