

#include <stm32f4xx.h>




const uint32_t GPIO_MODER_INPUT = 0b00;
const uint32_t GPIO_MODER_OUTPUT = 0b01;
const uint32_t GPIO_MODER_ALTERNATE = 0b10;
const uint32_t GPIO_MODER_ANALOG = 0b11;


void init()
{

    RCC->AHB1ENR |=
        RCC_AHB1ENR_GPIOCEN |
        RCC_AHB1ENR_GPIOBEN;

    GPIOC->MODER |=
        (GPIO_MODER_OUTPUT << GPIO_MODER_MODER0_Pos) |  // LD1 = PC0
        (GPIO_MODER_OUTPUT << GPIO_MODER_MODER1_Pos) |  // LD2 = PC1
        (GPIO_MODER_OUTPUT << GPIO_MODER_MODER2_Pos) |  // LD3 = PC2
        (GPIO_MODER_OUTPUT << GPIO_MODER_MODER3_Pos);   // LD4 = PC3

    GPIOB->MODER |=
        (GPIO_MODER_OUTPUT << GPIO_MODER_MODER0_Pos) |
        (GPIO_MODER_OUTPUT << GPIO_MODER_MODER1_Pos) |
        (GPIO_MODER_OUTPUT << GPIO_MODER_MODER2_Pos);


    // TODO: Use PLL to increase main clock frequency
    const uint32_t MAIN_CLOCK_FREQ = 8000000;
    // SysTick_Config(MAIN_CLOCK_FREQ / 1000);  // 1ms ticks
    SysTick_Config(250 * MAIN_CLOCK_FREQ / 1000);  // 250ms ticks

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


    GPIOC->BSRR =
        GPIO_BSRR_BS0 |
        GPIO_BSRR_BS1 |
        GPIO_BSRR_BS2 |
        GPIO_BSRR_BS3;

    GPIOB->ODR =
        GPIO_ODR_OD0 |
        GPIO_ODR_OD1 |
        GPIO_ODR_OD2;


    while (true)
    {

    }

}
