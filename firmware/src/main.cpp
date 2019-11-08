
// TODO: Abstract hadware stuff into a BSP file/directory,
// which I can swap out between development boards (or even devices, possibly)

#include <stm32f4xx.h>

#include <array>



extern "C" void SysTick_Handler(void)
{
    GPIOD->ODR ^= (1 << 0);
}


// Separate header for stuff like this
const uint32_t GPIO_MODER_ALTERNATE = 0x02;


void sysinit(void)
{
    // Init external clock and wait for external crystal to stabilize
    RCC->CR |= RCC_CR_HSEON;
    while ( ! (RCC->CR & RCC_CR_HSERDY));
    RCC->CFGR |=  (0x01 << 0);  // enable HSE oscillator  // RCC_CFGR_SW

    // Enable SYSCLK output on MCO2 (the default)
    RCC->CFGR |= (0x02 << 21);  // enable HSE clock on MCO1

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOCEN;
    RCC->APB1ENR |=
        RCC_APB1ENR_TIM2EN |    // enable TIM2
        RCC_APB1ENR_USART2EN |  // enable USART2
        RCC_APB1ENR_DACEN;

    RCC->APB2ENR |=
        RCC_APB2ENR_SPI1EN;     // enable SPI1



    // Configure USART2
    // See manual of Figure 26 for GPIO alt function mapping
    GPIOA->MODER |=
        (GPIO_MODER_ALTERNATE << 4) |  // TX=PA2
        (GPIO_MODER_ALTERNATE << 6);   // RX=PA3
    GPIOA->AFR[0] |=
        (7 << GPIO_AFRL_AFSEL2_Pos) |  // select USART2 AF (TX) for PA2
        (7 << GPIO_AFRL_AFSEL3_Pos);   // select USART2 AF (RX) for PA3

    USART2->CR1 =
        USART_CR1_UE;  // enable USART
        // USART_CR1 M,PCE unset  -> 1 start bit, 8 data bits, no parity check
        // USART_CR1_TXEIE |  // enable transmit buffer empty interrupt
        // USART_CR1_TCIE |  // transmit complete interrupt
        // USART_CR1_TE |  // enable transmitter
        // USART_CR1_RE;   // enable receiver

    // TODO: May have to get external crystal working first
    // For 9600 baud:  (16 MHz) / 8(208 + 5/16) = 9600.96
    // TODO: Extract to constexpr function or something.
    // USART2->BRR = ((208 << 4) & 0xfff0) | (5 & 0x000f);
    USART2->BRR = ((52 << 4) & 0xfff0) | (1 & 0x000f);



    GPIOA->MODER |=
        (GPIO_MODER_ALTERNATE << 16);  // MCO1 is on PA8

    GPIOC->MODER |=
        (GPIO_MODER_ALTERNATE << 18);  // MCO2 is on PC9  (7.2.10)

    // Configure LEDs as outputs
    GPIOD->MODER |=
        (0x01 << 0) |

        (0x01 << 24) |
        (0x01 << 26) |
        (0x01 << 28) |
        (0x01 << 30);

    GPIOD->MODER |=
        (0x01 << 0);
    GPIOD->BSRR = GPIO_BSRR_BS0;

    TIM2->PSC = 4000 - 1;
    TIM2->CCR1 = 250;
    TIM2->CCR2 = 500;
    TIM2->CCR3 = 750;
    TIM2->ARR = 1000;

    TIM2->DIER |=
        TIM_DIER_CC1IE |
        TIM_DIER_CC2IE |
        TIM_DIER_CC3IE |
        TIM_DIER_UIE;

    TIM2->CR1 |= TIM_CR1_CEN;  // Start timer





    // Setup DAC2  (PA5)

    // Configure USART2
    // See manual of Figure 26 for GPIO alt function mapping
    // GPIOA->MODER |=
        // (GPIO_MODER_ALTERNATE << GPIO_MODER_MODER6_Pos);  // DAC2_OUT=PA5
    // GPIOA->AFR[0] |= ???;

    DAC1->DHR12R2 = 0;

    DAC1->CR =
        DAC_CR_EN2 |
        DAC_CR_BOFF2 |
        // (0b111 << DAC_CR_TSEL2_Pos) |
        0 ;





    const uint32_t MAIN_CLOCK_FREQ = 8000000;
    SysTick_Config(MAIN_CLOCK_FREQ / 1000);  // 1ms ticks

    NVIC_EnableIRQ(TIM2_IRQn);

    __enable_irq();


}




// TODO: Use interrupt
// TODO: Move these to usart.cpp or something
void sendChar(char byte) {
    USART2->DR = static_cast<uint32_t>(byte);  // TODO: Add mask for 8 (?) bits
    GPIOD->BSRR = GPIO_BSRR_BS12;
    while ( ! (USART2->SR & USART_SR_TXE));
    GPIOD->BSRR = GPIO_BSRR_BR12;
}

void sendString(const char* string, uint32_t maxlen = 0xffffffff) {
    uint32_t charsSent = 0;
    while (*string != '\0' && charsSent < maxlen)
    {
        sendChar(static_cast<uint8_t>(*string++));
        charsSent += 1;
    }
}



void formatHex32(char* buffer, uint32_t value)
{
    const char* lookupTable = "0123456789abcdef";
    const std::array<uint8_t, 8> nibbles {
        static_cast<uint8_t>((value & 0xf0000000) >> 28),
        static_cast<uint8_t>((value & 0x0f000000) >> 24),
        static_cast<uint8_t>((value & 0x00f00000) >> 20),
        static_cast<uint8_t>((value & 0x000f0000) >> 16),
        static_cast<uint8_t>((value & 0x0000f000) >> 12),
        static_cast<uint8_t>((value & 0x00000f00) >> 8),
        static_cast<uint8_t>((value & 0x000000f0) >> 4),
        static_cast<uint8_t>((value & 0x0000000f) >> 0),
    };
    for (size_t i = 0; i < nibbles.size(); i++)
    {
        buffer[i] = lookupTable[nibbles[i]];
    }
    buffer[nibbles.size()] = '\0';
}


extern "C" void NMI_Handler()
{
    sendString("NMI_Handler \r\n");
    while(true);
}

extern "C" void HardFault_Handler()
{
    char buffer[16];
    formatHex32(buffer, SCB->HFSR);

    sendString("HardFault_Handler \r\n");
    sendString("  SCB->HFSR = 0x");
    sendString(buffer);
    sendString("\r\n");

    // http://blog.feabhas.com/2013/02/developing-a-generic-hard-fault-handler-for-arm-cortex-m3cortex-m4/
    if (SCB->HFSR & SCB_HFSR_FORCED_Msk)
    {
        formatHex32(buffer, SCB->CFSR);
        sendString("  SCB->CFSR = 0x");
        sendString(buffer);
        sendString("\r\n");

        if (SCB->CFSR & SCB_CFSR_BFARVALID_Msk) sendString("BFARVALID \r\n");
        if (SCB->CFSR & SCB_CFSR_LSPERR_Msk) sendString("LSPERR \r\n");
        if (SCB->CFSR & SCB_CFSR_STKERR_Msk) sendString("STKERR \r\n");
        if (SCB->CFSR & SCB_CFSR_UNSTKERR_Msk) sendString("UNSTKERR \r\n");
        if (SCB->CFSR & SCB_CFSR_IMPRECISERR_Msk) sendString("IMPRECISERR \r\n");
        if (SCB->CFSR & SCB_CFSR_PRECISERR_Msk) sendString("PRECISERR \r\n");
        if (SCB->CFSR & SCB_CFSR_IBUSERR_Msk) sendString("IBUSERR \r\n");

        if (SCB->CFSR & SCB_CFSR_DIVBYZERO_Msk) sendString("DIVBYZERO \r\n");
        if (SCB->CFSR & SCB_CFSR_UNALIGNED_Msk) sendString("UNALIGNED \r\n");
        if (SCB->CFSR & SCB_CFSR_NOCP_Msk) sendString("NOCP \r\n");
        if (SCB->CFSR & SCB_CFSR_INVPC_Msk) sendString("INVPC \r\n");
        if (SCB->CFSR & SCB_CFSR_INVSTATE_Msk) sendString("INVSTATE \r\n");
        if (SCB->CFSR & SCB_CFSR_UNDEFINSTR_Msk) sendString("UNDEFINSTR \r\n");
    }


    while(true);
}

extern "C" void MemManage_Handler()
{
    sendString("MemManage_Handler \r\n");
    while(true);
}

extern "C" void BusFault_Handler()
{
    sendString("BusFault_Handler \r\n");
    while(true);
}

extern "C" void UsageFault_Handler()
{
    sendString("UsageFault_Handler \r\n");
    while(true);
}






#include <printf.h>

extern "C" void _putchar(char character)
{
    // TODO: To support writing directly to UART
    // https://github.com/mpaland/printf#usage

    // send char to console etc.

    (void)character;
}



#include <cstring>

int main()
{
    sysinit();

    USART2->CR1 |= USART_CR1_TE;  // start transmit
    sendString("\r\nSTM32F4DISCOVERY Booted up!\r\n");
    sendString("===========================================\r\n");


    std::array<std::int_fast32_t, 8> values {1, 2, 4, 8, 16, 32, 64, 128};

    char buffer[256];
    ssize_t length {0};
    for (size_t i = 0; i < values.size(); i++)
    {
        char* cursor = buffer + length;
        size_t remaining = sizeof(buffer) - length;
        length += snprintf(cursor, remaining, "%d => %d \r\n", i, values[i]);
        // length += std::snprintf(cursor, remaining, "blah \r\n");
        // length += std::sprintf(cursor, "blah \r\n");

        // sprintf isn't working for some reason. Maybe I'll just use
        // my own that can print strings and ints. This is some bullshit.
        // It just hard faults (I think). Google isn't turning up anything

        // Look into using gdb to step into the code running on-target.
        // Maybe you can see where it's going wrong then. It could be
        // that the linker script is set up wrong.

    }
    buffer[length] = '\0';
    sendString(buffer);

    while (1)
    {
    }

}


extern "C" void TIM2_IRQHandler(void)
{
    auto setLeds = [](bool green, bool red, bool blue, bool orange) {
        const uint32_t value {
            (green ? GPIO_BSRR_BS12 : GPIO_BSRR_BR12) |
            (orange ? GPIO_BSRR_BS13 : GPIO_BSRR_BR13) |
            (red ? GPIO_BSRR_BS14 : GPIO_BSRR_BR14) |
            (blue ? GPIO_BSRR_BS15 : GPIO_BSRR_BR15)
        };
        GPIOD->BSRR = value;
    };

    // const bool useCirclePattern = false;
    if (TIM2->SR & TIM_SR_CC1IF)
    {
        // if (useCirclePattern)
        // {
        //     GPIOD->BSRR =
        //         GPIO_BSRR_BS12 |
        //         GPIO_BSRR_BR13 | GPIO_BSRR_BR14 | GPIO_BSRR_BR15;
        // }
        // else
        {
            setLeds(true, true, false, false);
        }
        DAC1->DHR12R2 = 0;  // 0V
        TIM2->SR &= ~TIM_SR_CC1IF;
    }
    else if (TIM2->SR & TIM_SR_CC2IF)
    {
        // if (useCirclePattern)
        // {
        //     GPIOD->BSRR =
        //         GPIO_BSRR_BS13 |
        //         GPIO_BSRR_BR12 | GPIO_BSRR_BR14 | GPIO_BSRR_BR15;
        // }
        DAC1->DHR12R2 = 1241;  // 1V
        TIM2->SR &= ~TIM_SR_CC2IF;
    }
    else if (TIM2->SR & TIM_SR_CC3IF)
    {
        // if (useCirclePattern)
        // {
        //     GPIOD->BSRR =
        //         GPIO_BSRR_BS14 |
        //         GPIO_BSRR_BR12 | GPIO_BSRR_BR13 | GPIO_BSRR_BR15;
        // }
        // else
        {
            setLeds(false, false, true, true);
        }
        DAC1->DHR12R2 = 2482;  // 2V
        TIM2->SR &= ~TIM_SR_CC3IF;
    }
    else if (TIM2->SR & TIM_SR_UIF)
    {
        // if (useCirclePattern)
        // {
        //     GPIOD->BSRR =
        //         GPIO_BSRR_BS15 |
        //         GPIO_BSRR_BR12 | GPIO_BSRR_BR13 | GPIO_BSRR_BR14;
        // }
        DAC1->DHR12R2 = 3724;  // 3V
        TIM2->SR &= ~TIM_SR_UIF;
    }

}
