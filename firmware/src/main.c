
// TODO: Abstract hadware stuff into a BSP file/directory,
// which I can swap out between development boards (or even devices, possibly)

#include <stdbool.h>

#include <stm32f4xx.h>



// // A 32-bit counter allows for about 50 days of millisecond counting.
// // A 64-bit counter would allow for 213 billion days of millisecond counting.
// volatile uint32_t ticks = 2;
// void SysTick_Handler(void)
// {
//     ticks += 1;

//     GPIOD->BSRR = (ticks % 2 == 0) ? GPIO_BSRR_BS0 : GPIO_BSRR_BR0;

//     // if (ticks % 250 == 0)
//     // {
//     //     presses += 1;
//     // }
// }



volatile uint32_t presses = 2;


// A 32-bit counter allows for about 50 days of millisecond counting.
// A 64-bit counter would allow for 213 billion days of millisecond counting.
// volatile uint32_t ticks = 2;
volatile uint32_t ticks;
void SysTick_Handler(void)
{
    // ticks += 1;

    // // GPIOD->BSRR = (ticks % 2 == 0) ? GPIO_BSRR_BS0 : GPIO_BSRR_BR0;


    // static bool blueOn = false;
    // if (ticks++ >= 1000)
    // {
    //     ticks = 0;
    //     blueOn = ! blueOn;
    // }

    // GPIOD->BSRR = blueOn ? GPIO_BSRR_BS15 : GPIO_BSRR_BR15;

    GPIOD->BSRR = GPIO_BSRR_BR15;

    GPIOD->ODR ^= (1 << 0);
    // GPIOD->BSRR = GPIO_BSRR_BS0;



    // GPIOD->BSRR = blueOn ? GPIO_BSRR_BS15 : GPIO_BSRR_BR15;
    // GPIOD->BSRR = blueOn ? GPIO_BSRR_BR13 : GPIO_BSRR_BS13;

    // GPIOD->BSRR = GPIO_BSRR_BR15;


    // if (ticks % 250 == 0)
    // {
    //     presses += 1;
    // }
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
    // RCC->CFGR |= (00)
    RCC->CFGR |= (0x02 << 21);  // enable HSE clock on MCO1


    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOCEN;
    RCC->APB1ENR |=
        RCC_APB1ENR_TIM2EN |    // enable TIM2
        RCC_APB1ENR_USART2EN;   // enable USART2
    RCC->APB2ENR |=
        RCC_APB2ENR_SPI1EN;     // enable SPI1


    // // Configure USART2
    // GPIOA->MODER |=
    //     (GPIO_MODER_ALTERNATE << 4) |  // TX=PA2
    //     (GPIO_MODER_ALTERNATE << 6);   // RX=PA3
    // GPIOA->AFRL |=
    //     (7 << GPIO_AFRL2) |
    //     (7 << GPIO_AFRL3);



    // USART2->CR1 =
    //     USART_CR1_UE;  // enable USART
    //     // USART_CR1 M,PCE unset  -> 1 start bit, 8 data bits, no parity check
    //     // USART_CR1_TXEIE |  // enable transmit buffer empty interrupt
    //     // USART_CR1_TCIE |  // transmit complete interrupt
    //     // USART_CR1_TE |  // enable transmitter
    //     // USART_CR1_RE;   // enable receiver

    // // TODO: May have to get external crystal working first
    // // For 9600 baud:  (16 MHz) / 8(208 + 5/16) = 9600.96
    // // TODO: Extract to constexpr function or something.
    // // USART2->BRR = ((208 << 4) & 0xfff0) | (5 & 0x000f);
    // USART2->BRR = ((52 << 4) & 0xfff0) | (1 & 0x000f);




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

    // TIM2->CCR1 = counterValue / 4;
    // TIM2->CCR2 = TIM2->CCR1 * 2;
    // TIM2->CCR3 = TIM2->CCR1 * 3;
    // TIM2->ARR = TIM2->CCR1 * 4;

    TIM2->DIER |=
        TIM_DIER_CC1IE |
        TIM_DIER_CC2IE |
        TIM_DIER_CC3IE |
        TIM_DIER_UIE;

    TIM2->CR1 |= TIM_CR1_CEN;  // Start timer




    // GPIOA->MODER |=
    //     (GPIO_MODER_ALTERNATE << (2 * 5)) |     // PA5 -> SCK (SPI1)        "alternate"
    //     (GPIO_MODER_ALTERNATE << (2 * 6)) |     // PA6 -> MISO (SPI1)   "alternate"
    //     (GPIO_MODER_ALTERNATE << (2 * 7));      // PA7 -> MOSI (SPI1)   "alternate"
    // GPIOA->AFR[0] |=
    //     (5 << GPIO_AFRL_AFSEL5_Pos) |
    //     (5 << GPIO_AFRL_AFSEL6_Pos) |
    //     (5 << GPIO_AFRL_AFSEL7_Pos);



    // SPI1->CR1 |=
    //     0b111 << SPI_CR1_BR |   // SPI baud rate = f_pclk / 256 (31.25 kHz?)
    //     // SPI_CR1_DFF |           // 16-bit data frame format
    //     SPI_CR1_LSBFIRST |      // least-significant bit first
    //     SPI_CR1_MSTR;           // Configure as SPI master


    // SPI1->CR1 |= SPI_CR1_SPE;  // Enable SPI peripheral




    // Increment a counter every millisecond
    // GPIOD->BSRR = GPIO_BSRR_BS13;
    // if (SysTick_Config(8000) == 0)
    // {
    //     // Success
    //     GPIOD->BSRR = GPIO_BSRR_BR13;
    // }





 //  SysTick->LOAD  = (uint32_t)(8000 - 1UL);                         /* set reload register */
 //  NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); /* set Priority for Systick Interrupt */
 //  SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
 // SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
 //                   SysTick_CTRL_TICKINT_Msk   |
 //                   SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */



    const uint32_t MAIN_CLOCK_FREQ = 8000000;
    SysTick_Config(MAIN_CLOCK_FREQ / 1000);  // 1ms ticks

    NVIC_EnableIRQ(TIM2_IRQn);

    __enable_irq();






}




volatile bool waiting = true;




int main()
{
    ticks = 0;

    sysinit();

    GPIOD->BSRR = GPIO_BSRR_BS15;


    while (1)
    {
    }

}





void TIM2_IRQHandler(void)
{
    // auto setLeds = [](bool green, bool red, bool blue, bool orange) {
    //     uint32_t value = 0;
    //     if (counterValue == 3330)
    //     {
    //         value |= green ? GPIO_BSRR_BS12 : GPIO_BSRR_BR12;
    //         value |= orange ? GPIO_BSRR_BS13 : GPIO_BSRR_BR13;
    //         value |= red ? GPIO_BSRR_BS14 : GPIO_BSRR_BR14;
    //         value |= blue ? GPIO_BSRR_BS15 : GPIO_BSRR_BR15;
    //     }
    //     else
    //     {
    //         value = GPIO_BSRR_BS12 |GPIO_BSRR_BS13 | GPIO_BSRR_BS14 | GPIO_BSRR_BS15;
    //     }
    //     GPIOD->BSRR = value;
    // };

    // // const bool useCirclePattern = presses % 2 == 0;
    // if (TIM2->SR & TIM_SR_CC1IF)
    // {
    //     // if (useCirclePattern)
    //     // {
    //     //     GPIOD->BSRR =
    //     //         GPIO_BSRR_BS12 |
    //     //         GPIO_BSRR_BR13 | GPIO_BSRR_BR14 | GPIO_BSRR_BR15;
    //     // }
    //     // else
    //     // {
    //     //     setLeds(true, true, false, false);
    //     // }
    //     TIM2->SR &= ~TIM_SR_CC1IF;
    // }
    // else if (TIM2->SR & TIM_SR_CC2IF)
    // {
    //     GPIOD->BSRR = GPIO_BSRR_BS15;
    //     // if (useCirclePattern)
    //     // {
    //     //     GPIOD->BSRR =
    //     //         GPIO_BSRR_BS13 |
    //     //         GPIO_BSRR_BR12 | GPIO_BSRR_BR14 | GPIO_BSRR_BR15;
    //     // }
    //     TIM2->SR &= ~TIM_SR_CC2IF;
    // }
    // else if (TIM2->SR & TIM_SR_CC3IF)
    // {
    //     // if (useCirclePattern)
    //     // {
    //     //     GPIOD->BSRR =
    //     //         GPIO_BSRR_BS14 |
    //     //         GPIO_BSRR_BR12 | GPIO_BSRR_BR13 | GPIO_BSRR_BR15;
    //     // }
    //     // else
    //     // {
    //     //     setLeds(false, false, true, true);
    //     // }
    //     TIM2->SR &= ~TIM_SR_CC3IF;
    // }
    // else if (TIM2->SR & TIM_SR_UIF)
    // {
    //     waiting = false;
    //     GPIOD->BSRR = GPIO_BSRR_BR15;
    //     // if (useCirclePattern)
    //     // {
    //     //     GPIOD->BSRR =
    //     //         GPIO_BSRR_BS15 |
    //     //         GPIO_BSRR_BR12 | GPIO_BSRR_BR13 | GPIO_BSRR_BR14;
    //     // }
    //     TIM2->SR &= ~TIM_SR_UIF;
    // }




    // const bool useCirclePattern = presses % 2 == 0;
    if (TIM2->SR & TIM_SR_CC1IF)
    {
        static bool green = true;
        green = ! green;

        if (green)
        {
            GPIOD->BSRR = GPIO_BSRR_BS12 | GPIO_BSRR_BR14;
        }
        else
        {
            GPIOD->BSRR = GPIO_BSRR_BR12 | GPIO_BSRR_BS14;
        }

        // GPIOD->BSRR = green ? GPIO_BSRR_BS15 : GPIO_BSRR_BR15;
        // GPIOD->BSRR = green ? GPIO_BSRR_BR13 : GPIO_BSRR_BS13;


        TIM2->SR &= ~TIM_SR_CC1IF;
    }
    else if (TIM2->SR & TIM_SR_CC2IF)
    {
        // GPIOD->BSRR = GPIO_BSRR_BS15;
        // if (useCirclePattern)
        // {
        //     GPIOD->BSRR =
        //         GPIO_BSRR_BS13 |
        //         GPIO_BSRR_BR12 | GPIO_BSRR_BR14 | GPIO_BSRR_BR15;
        // }
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
        // {
        //     setLeds(false, false, true, true);
        // }
        TIM2->SR &= ~TIM_SR_CC3IF;
    }
    else if (TIM2->SR & TIM_SR_UIF)
    {
        // waiting = false;
        // GPIOD->BSRR = GPIO_BSRR_BR15;
        // if (useCirclePattern)
        // {
        //     GPIOD->BSRR =
        //         GPIO_BSRR_BS15 |
        //         GPIO_BSRR_BR12 | GPIO_BSRR_BR13 | GPIO_BSRR_BR14;
        // }
        TIM2->SR &= ~TIM_SR_UIF;
    }





}
