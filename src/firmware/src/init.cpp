
#include <cstdint>

#include <stm32f4xx.h>
#include "init.hpp"


const std::uint32_t GPIO_MODER_INPUT = 0b00;
const std::uint32_t GPIO_MODER_OUTPUT = 0b01;
const std::uint32_t GPIO_MODER_ALTERNATE = 0b10;
const std::uint32_t GPIO_MODER_ANALOG = 0b11;


namespace octane
{


void init()
{
    // Init external clock and wait for external crystal to stabilize
    RCC->CR |= RCC_CR_HSEON;
    while ( ! (RCC->CR & RCC_CR_HSERDY));
    RCC->CFGR |= (0x01 << 0);  // enable HSE oscillator  // RCC_CFGR_SW


    RCC->AHB1ENR |=
        RCC_AHB1ENR_GPIOAEN |
        RCC_AHB1ENR_GPIOBEN |
        RCC_AHB1ENR_GPIOCEN;

    RCC->APB2ENR |=
        RCC_APB2ENR_SPI1EN |
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
        (7 << GPIO_AFRL_AFSEL6_Pos) |  // USART1.TX = PB6
        (7 << GPIO_AFRL_AFSEL7_Pos);   // USART1.RX = PB7



    // Set up SPI Flash interface

    // PA4: /FLASH_SPI_CE
    // PA5: FLASH_SPI_SCK
    // PA6: FLASH_SPI_MISO
    // PA7: FLASH_SPI_MOSI
    // PC4: /FLASH_SPI_WP
    // PC5: /FLASH_SPI_HOLD


    GPIOA->MODER |=
        (GPIO_MODER_OUTPUT << GPIO_MODER_MODER4_Pos) |     // /FLASH_SPI_CE = PA4
        (GPIO_MODER_ALTERNATE << GPIO_MODER_MODER5_Pos) |  // FLASH_SPI_SCK = PA5
        (GPIO_MODER_ALTERNATE << GPIO_MODER_MODER6_Pos) |  // FLASH_SPI_MISO = PA6
        (GPIO_MODER_ALTERNATE << GPIO_MODER_MODER7_Pos);   // FLASH_SPI_MOSI = PA7

    GPIOA->AFR[0] |=
        (5 << GPIO_AFRL_AFSEL5_Pos) |  // SPI1.SCK = PA5
        (5 << GPIO_AFRL_AFSEL6_Pos) |  // SPI1.MISO = PA6
        (5 << GPIO_AFRL_AFSEL7_Pos);   // SPI1.MOSI = PA7

    GPIOC->MODER |=
        (GPIO_MODER_OUTPUT << GPIO_MODER_MODER4_Pos) |  // /FLASH_SPI_WP = PC4
        (GPIO_MODER_OUTPUT << GPIO_MODER_MODER5_Pos);   // /FLASH_SPI_HOLD = PC5


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
    SysTick_Config(MAIN_CLOCK_FREQ / 1000);  // 1ms ticks

    NVIC_EnableIRQ(SPI1_IRQn);
    NVIC_EnableIRQ(SPI2_IRQn);
    NVIC_EnableIRQ(SPI5_IRQn);


}


}
