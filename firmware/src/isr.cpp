

#include <printf.h>
#include <stm32f4xx.h>


extern "C" void SysTick_Handler()
{

}



// The system freezes when this interrupt triggers for some reason...

// volatile uint16_t lastSample;
// volatile bool fpgaSpiReady = true;
extern "C" void SPI2_Handler()
{
    if (SPI2->SR & SPI_SR_RXNE)
    {
        // lastSample = SPI2->DR;
    }
    else if (SPI2->SR & SPI_SR_TXE)
    {
        // fpgaSpiReady = true;

        // this is wrong
        GPIOB->BSRR = GPIO_BSRR_BS12;  // pull NSS high

    }


}



volatile bool fpgaLedOn = false;

extern "C" void TIM2_IRQHandler()
{
    if (TIM2->SR & TIM_SR_CC1IF)
    {
        GPIOD->BSRR = GPIO_BSRR_BS13;
        TIM2->SR &= ~TIM_SR_CC1IF;
    }
    else if (TIM2->SR & TIM_SR_CC2IF)
    {
        fpgaLedOn = true;
        TIM2->SR &= ~TIM_SR_CC2IF;
    }
    else if (TIM2->SR & TIM_SR_CC3IF)
    {
        GPIOD->BSRR = GPIO_BSRR_BR13;
        TIM2->SR &= ~TIM_SR_CC3IF;
    }
    else if (TIM2->SR & TIM_SR_UIF)
    {
        fpgaLedOn = false;
        TIM2->SR &= ~TIM_SR_UIF;
    }
}


extern uint16_t lastSample;
volatile bool newSampleAvailable = false;
volatile uint16_t currentSample;
extern "C" void TIM3_IRQHandler()
{
    if (TIM3->SR & TIM_SR_UIF)
    {
        newSampleAvailable = true;
        currentSample = lastSample;
        // DAC1->DHR12R2 = currentSample >> 8;
        TIM3->SR &= ~TIM_SR_UIF;
    }
}




extern "C" void HardFault_Handler()
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
