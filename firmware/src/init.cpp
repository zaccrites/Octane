
#include "init.hpp"
#include <stm32f4xx.h>



const uint32_t GPIO_MODER_INPUT = 0b00;
const uint32_t GPIO_MODER_OUTPUT = 0b01;
const uint32_t GPIO_MODER_ALTERNATE = 0b10;
const uint32_t GPIO_MODER_ANALOG = 0b11;




void octane::init()
{
    // Init external clock and wait for external crystal to stabilize
    RCC->CR |= RCC_CR_HSEON;
    while ( ! (RCC->CR & RCC_CR_HSERDY));
    RCC->CFGR |=  (0x01 << 0);  // enable HSE oscillator  // RCC_CFGR_SW


    RCC->AHB1ENR |=
        RCC_AHB1ENR_GPIOAEN |
        RCC_AHB1ENR_GPIOBEN |
        RCC_AHB1ENR_GPIODEN |
        RCC_AHB1ENR_GPIOCEN |
        RCC_AHB1ENR_DMA1EN;

    RCC->APB1ENR |=
        RCC_APB1ENR_TIM2EN |    // enable TIM2
        RCC_APB1ENR_TIM3EN |    // enable TIM3
        RCC_APB1ENR_USART2EN |  // enable USART2
        RCC_APB1ENR_SPI2EN |
        RCC_APB1ENR_DACEN;


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

    USART2->CR1 |= USART_CR1_TE;  // start transmit



    // Configure LEDs as outputs
    GPIOD->MODER |=
        (0x01 << 24) |
        (0x01 << 26) |
        (0x01 << 28) |
        (0x01 << 30);

    GPIOD->MODER |=
        (0x01 << 0);


    // TIM2->PSC = 4000 - 1;
    TIM2->PSC = 8000 - 1;
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


    // TIM3 will just trigger its interrupt at 46.875 kHz (though this comes to more like 46.516 kHz)
    TIM3->PSC = 0;
    TIM3->ARR = 171;  // If I clock the MCU timer higher, then I can get a more accurate sample clock
    TIM3->DIER = TIM_DIER_UIE;
    TIM3->CR1 = TIM_CR1_CEN;  // Start timer


    // Setup DAC2  (PA5)
    GPIOA->MODER |=
        (0b11 << GPIO_MODER_MODER5_Pos);  // DAC2_OUT=PA5
    DAC1->DHR12R2 = 0;
    // DAC1->CR = DAC_CR_EN2 | DAC_CR_BOFF2;
    DAC1->CR = DAC_CR_EN2;


    // Setup SPI2 to talk to FPGA
    // For now just bit-bang it
    // SPI2_SCK = PB13
    // SPI2_MISO = PB14
    // SPI2_MOSI = PB15
    //
    // TODO: Use alternate function for these pins so that the SPI hardware takes over
    GPIOB->MODER |=
        (GPIO_MODER_OUTPUT << GPIO_MODER_MODER12_Pos) |  // NSS
        (GPIO_MODER_ALTERNATE << GPIO_MODER_MODER13_Pos) |  // SCK
        (GPIO_MODER_ALTERNATE << GPIO_MODER_MODER14_Pos) |  // MISO
        (GPIO_MODER_ALTERNATE << GPIO_MODER_MODER15_Pos) |  // MOSI
        (GPIO_MODER_OUTPUT << GPIO_MODER_MODER8_Pos);  // FPGA_RESET is an output

    // GPIOB->OTYPER |=
    //     GPIO_OTYPER_OT12;  // Set NSS as open-drain pin, pulled up by external resistor

    GPIOB->AFR[1] |=
        // (5 << GPIO_AFRH_AFSEL12_Pos) |
        (5 << GPIO_AFRH_AFSEL13_Pos) |
        (5 << GPIO_AFRH_AFSEL14_Pos) |
        (5 << GPIO_AFRH_AFSEL15_Pos);



    // The FPGA acts on a rising SCK edge, both outputting the next MISO
    // bit and sampling the current MOSI bit.
    //
    // I think that this means that SCK should be low at idle (CPOL=0, the default)
    // and that SCK phase should use the first edge (CPHA=0, the default),
    // which corresponds to the rising edge when CPOL is reset (according to section
    // 28.3.1 "Clock phase and clock polarity" of the STM32F4xx reference manual).

    // SPI2->CR1 =
    //     SPI_CR1_MSTR |  // act as SPI master
    //     // (0b001 << SPI_CR1_BR_Pos) |  // set baud rate to f_PCLK / 4 (2 MHz)
    //     (0b011 << SPI_CR1_BR_Pos) |  // set baud rate to f_PCLK / 16
    //     SPI_CR1_DFF;   // use 16 bit frame, MSB out first
    //     // SPI_CR1_SSI | SPI_CR1_SSM;    // software slave management

    // SPI2->CR2 =
    //     SPI_CR2_FRF |  // ???
    //     SPI_CR2_SSOE |  // drive NSS low when communicating with slave
    //     SPI_CR2_RXNEIE;  // trigger interrupt when data is received
    //     // TODO: Use TX/RX buffer DMA enable


    // The glitchiness seems to just get WORSE the slower the SPI clock is!
    SPI2->CR1 =
        SPI_CR1_MSTR |  // act as SPI master
        (0b011 << SPI_CR1_BR_Pos) |
        // (0b010 << SPI_CR1_BR_Pos) |  // set baud rate to f_PCLK / 8 (500 kHz)
        // (0b001 << SPI_CR1_BR_Pos) |  // set baud rate to f_PCLK / 4 (2 MHz)  // HARD FAULT, for some reason (PRECISERR, BFARVALID)
        SPI_CR1_DFF |   // use 16 bit frame, MSB out first
        SPI_CR1_CPHA |
        SPI_CR1_SSI | SPI_CR1_SSM;    // software slave management


    // The interrupts aren't enabled
    SPI2->CR2 = 0;

        // SPI_CR2_TXEIE;

        // SPI_CR2_FRF |  // use TI mode
        // SPI_CR2_SSOE |  // drive NSS low when communicating with slave
        // SPI_CR2_TXEIE | // trigger interrupt when data is transmitted
        // SPI_CR2_RXNEIE;  // trigger interrupt when data is received
        // TODO: Use TX/RX buffer DMA enable

    // Data is valid on the falling edge of SCK
    // Synchronize on the falling edge of SCK when NSS is high



    // Use DMA1 to handle SPI2 transmission
    // (Stream 4, Channel 0 for Tx)
    // DMA1_Stream4->
    // (Stream 3, Channel 0 for Rx)

    // Before transfer...
    // TODO: enable interrupts

    // DMA1_Stream4->NDTR = COUNT;  // half words (commands * 2)
    // DMA1_Stream4->M0AR = <source buffer>;
    // DMA1_Stream4->PAR = <peripheral address>;  ?
    // DMA1_Stream4->PSIZE = <half word>? or <word>, unless address and value writes are separate?

    // DMA1_Stream4->CR |=
    //     (0b10 << DMA_SxCR_PL) |         // "high priority"
    //     (0 << DMA_SxCR_CHSEL_Pos) |   // use Channel 0 (SPI Tx)
    //     (0b01 << DMA_SxCR_DIR_Pos) |  // memory-to-peripheral (SPI Tx)

    //     (0b10 << DMA_SxCR_PSIZE_Pos) |  // use half-word memory data size
    //     (0b10 << DMA_SxCR_MSIZE_Pos) |  // use half-word memory data size
    //     DMA_SxCR_MINC |     // increment memory pointer after each data transfer

    //     // I think peripheral increment is wrong, since all writes are done
    //     // to the DR register.

    //     // ...
    //     // ...
    //     DMA_SxCR_EN;                    // enable stream

    // // Note:   Before setting EN bit to '1' to start a new transfer,
    // // the event flags corresponding to the stream in DMA_LISR or DMA_HISR register must be cleared.








    // TODO: Use PLL to increase main clock frequency
    const uint32_t MAIN_CLOCK_FREQ = 8000000;
    SysTick_Config(MAIN_CLOCK_FREQ / 1000);  // 1ms ticks

    // TODO: NVIC_SetPriotityGroup, NVIC_EncodePriority?
    // https://www.keil.com/pack/doc/cmsis/Core/html/group__NVIC__gr.html

    // NVIC_SetPriority(TIM2_IRQn, 2);
    NVIC_SetPriority(TIM2_IRQn, 1);
    NVIC_EnableIRQ(TIM2_IRQn);

    // NVIC_SetPriority(TIM3_IRQn, 2);
    NVIC_SetPriority(TIM3_IRQn, 1);
    NVIC_EnableIRQ(TIM3_IRQn);

    // NVIC_SetPriority(SPI2_IRQn, 1);
    NVIC_SetPriority(SPI2_IRQn, 2);
    NVIC_EnableIRQ(SPI2_IRQn);

}
