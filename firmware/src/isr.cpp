

#include <printf.h>
#include <stm32f4xx.h>

#include "Fpga.hpp"


extern "C" void SysTick_Handler()
{

}



// The system freezes when this interrupt triggers for some reason...

// volatile uint16_t lastSample;
// volatile bool fpgaSpiReady = true;
extern "C" void SPI2_IRQHandler()
{
    if (SPI2->SR & SPI_SR_TXE)
    {
        octane::Fpga::getInstance().onSpiTxComplete();
    }
    if (SPI2->SR & SPI_SR_RXNE)
    {
        octane::Fpga::getInstance().onSpiRxComplete();
    }
}




volatile bool fpgaLedOn = false;

extern "C" void TIM2_IRQHandler()
{
    if (TIM2->SR & TIM_SR_CC1IF)
    {
        // GPIOD->BSRR = GPIO_BSRR_BS13;
        TIM2->SR &= ~TIM_SR_CC1IF;
    }
    else if (TIM2->SR & TIM_SR_CC2IF)
    {
        fpgaLedOn = true;
        TIM2->SR &= ~TIM_SR_CC2IF;
    }
    else if (TIM2->SR & TIM_SR_CC3IF)
    {
        // GPIOD->BSRR = GPIO_BSRR_BR13;
        TIM2->SR &= ~TIM_SR_CC3IF;
    }
    else if (TIM2->SR & TIM_SR_UIF)
    {
        fpgaLedOn = false;
        TIM2->SR &= ~TIM_SR_UIF;
    }
}



volatile bool updateSample;
extern "C" void TIM3_IRQHandler()
{
    if (TIM3->SR & TIM_SR_UIF)
    {
        updateSample = true;
        TIM3->SR &= ~TIM_SR_UIF;
    }
}




extern "C" void Real_HardFault_Handler(uint32_t* registers)
{

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





// // Listed below are the "unused" interrupt handlers.
// // The standard way to handle this is to use a weak assocaition to a
// // default handler defined in vectors.s, but this is a problem if
// // you add a real handler for an interrupt and misspell it.
// // Rather than the linker or compiler warning you about undefined or
// // multiply defined functions, it will instead just silently
// // not call your interrupt service routine.

// #define DEFAULT_HANDLER(int_name)  extern "C" void ...

// TODO: Define them in the cpp file to prevent this issue?
