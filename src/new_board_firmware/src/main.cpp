

#include <stm32f4xx.h>

// TODO: Wrap in a class which abstracts the output USART/SPI/whatever?
#include <printf.h>



const uint32_t GPIO_MODER_INPUT = 0b00;
const uint32_t GPIO_MODER_OUTPUT = 0b01;
const uint32_t GPIO_MODER_ALTERNATE = 0b10;
const uint32_t GPIO_MODER_ANALOG = 0b11;


void init()
{

    // Init external clock and wait for external crystal to stabilize
    RCC->CR |= RCC_CR_HSEON;
    while ( ! (RCC->CR & RCC_CR_HSERDY));
    RCC->CFGR |=  (0x01 << 0);  // enable HSE oscillator  // RCC_CFGR_SW


    RCC->AHB1ENR |=
        RCC_AHB1ENR_GPIOCEN |
        RCC_AHB1ENR_GPIOBEN;

    RCC->APB2ENR |=
        RCC_APB2ENR_USART1EN;

    GPIOC->MODER |=
        (GPIO_MODER_OUTPUT << GPIO_MODER_MODER0_Pos) |  // LD1 = PC0
        (GPIO_MODER_OUTPUT << GPIO_MODER_MODER1_Pos) |  // LD2 = PC1
        (GPIO_MODER_OUTPUT << GPIO_MODER_MODER2_Pos) |  // LD3 = PC2
        (GPIO_MODER_OUTPUT << GPIO_MODER_MODER3_Pos);   // LD4 = PC3

    GPIOB->MODER |=
        (GPIO_MODER_OUTPUT << GPIO_MODER_MODER0_Pos) |
        (GPIO_MODER_OUTPUT << GPIO_MODER_MODER1_Pos) |
        (GPIO_MODER_OUTPUT << GPIO_MODER_MODER2_Pos);


    GPIOB->MODER |=
        (GPIO_MODER_ALTERNATE << GPIO_MODER_MODER6_Pos) |  // USART1.TX = PB6
        (GPIO_MODER_ALTERNATE << GPIO_MODER_MODER7_Pos);   // USART1.RX = PB7

    GPIOB->AFR[0] =
        (6 << GPIO_AFRL_AFSEL6_Pos) |  // USART1.TX = PB6
        (7 << GPIO_AFRL_AFSEL6_Pos);   // USART1.RX = PB7


    USART1->CR1 =
        USART_CR1_UE;  // enable USART
        // USART_CR1 M,PCE unset  -> 1 start bit, 8 data bits, no parity check
        // USART_CR1_TXEIE |  // enable transmit buffer empty interrupt
        // USART_CR1_TCIE |  // transmit complete interrupt
        // USART_CR1_TE |  // enable transmitter
        // USART_CR1_RE;   // enable receiver

    // See Table 190 in manual, S.No=3 for 24 MHz
    USART1->BRR = (156 << 4) | 0b0100;  // 156.25
    USART1->CR1 |= USART_CR1_TE;  // start transmit


    // TODO: Use PLL to increase main clock frequency
    // SysTick_Config(MAIN_CLOCK_FREQ / 1000);  // 1ms ticks
    SysTick_Config(MAIN_CLOCK_FREQ / 4);  // 250ms ticks
    // SysTick_Config(MAIN_CLOCK_FREQ / 20);  // 250ms ticks

}



volatile uint32_t counter = 0;
extern "C" void SysTick_Handler()
{
    counter = (counter + 1) % 4;
    if      (counter == 0) GPIOC->BSRR = GPIO_BSRR_BS0 | GPIO_BSRR_BR1 | GPIO_BSRR_BR2 | GPIO_BSRR_BR3;
    else if (counter == 1) GPIOC->BSRR = GPIO_BSRR_BR0 | GPIO_BSRR_BS1 | GPIO_BSRR_BR2 | GPIO_BSRR_BR3;
    else if (counter == 2) GPIOC->BSRR = GPIO_BSRR_BR0 | GPIO_BSRR_BR1 | GPIO_BSRR_BS2 | GPIO_BSRR_BR3;
    else                   GPIOC->BSRR = GPIO_BSRR_BR0 | GPIO_BSRR_BR1 | GPIO_BSRR_BR2 | GPIO_BSRR_BS3;
}





void main()
{



    init();


    printf("Hello World! \r\n");


    bool ready = true;
    while (true)
    {

        if (counter == 0)
        {
            if (ready)
            {
                printf("Another cycle! \r\n");
                ready = false;
            }
        }
        else
        {
            ready = true;
        }

    }

}
