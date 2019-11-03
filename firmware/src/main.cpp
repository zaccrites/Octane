
// TODO: Abstract hadware stuff into a BSP file/directory,
// which I can swap out between development boards (or even devices, possibly)

#include <stm32f4xx.h>

#include <array>


static uint32_t index = 0;
static uint32_t counter = 0;
extern "C" void SysTick_Handler(void)
{
    std::array<uint16_t, 4> counts {1, 2, 4, 8};
    if (counter == 0)
    {
        index = (index + 1) % counts.size();
        counter = counts[index];

        GPIOD->ODR ^= (1 << 0);
    }
    else
    {
        counter = counter - 1;
    }
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
        RCC_APB1ENR_USART2EN;   // enable USART2
    RCC->APB2ENR |=
        RCC_APB2ENR_SPI1EN;     // enable SPI1

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

    const uint32_t MAIN_CLOCK_FREQ = 8000000;
    SysTick_Config(MAIN_CLOCK_FREQ / 1000);  // 1ms ticks

    NVIC_EnableIRQ(TIM2_IRQn);

    __enable_irq();

}


int main()
{
    sysinit();

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

    const bool useCirclePattern = false;
    if (TIM2->SR & TIM_SR_CC1IF)
    {
        if (useCirclePattern)
        {
            GPIOD->BSRR =
                GPIO_BSRR_BS12 |
                GPIO_BSRR_BR13 | GPIO_BSRR_BR14 | GPIO_BSRR_BR15;
        }
        else
        {
            setLeds(true, true, false, false);
        }
        TIM2->SR &= ~TIM_SR_CC1IF;
    }
    else if (TIM2->SR & TIM_SR_CC2IF)
    {
        if (useCirclePattern)
        {
            GPIOD->BSRR =
                GPIO_BSRR_BS13 |
                GPIO_BSRR_BR12 | GPIO_BSRR_BR14 | GPIO_BSRR_BR15;
        }
        TIM2->SR &= ~TIM_SR_CC2IF;
    }
    else if (TIM2->SR & TIM_SR_CC3IF)
    {
        if (useCirclePattern)
        {
            GPIOD->BSRR =
                GPIO_BSRR_BS14 |
                GPIO_BSRR_BR12 | GPIO_BSRR_BR13 | GPIO_BSRR_BR15;
        }
        else
        {
            setLeds(false, false, true, true);
        }
        TIM2->SR &= ~TIM_SR_CC3IF;
    }
    else if (TIM2->SR & TIM_SR_UIF)
    {
        if (useCirclePattern)
        {
            GPIOD->BSRR =
                GPIO_BSRR_BS15 |
                GPIO_BSRR_BR12 | GPIO_BSRR_BR13 | GPIO_BSRR_BR14;
        }
        TIM2->SR &= ~TIM_SR_UIF;
    }

}
