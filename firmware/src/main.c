
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
volatile uint32_t ticks = 2;
void SysTick_Handler(void)
{
    ticks += 1;

    GPIOD->BSRR = (ticks % 2 == 0) ? GPIO_BSRR_BS0 : GPIO_BSRR_BR0;

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
    SysTick_Config(8000);

    NVIC_EnableIRQ(TIM2_IRQn);
    __enable_irq();
}




volatile bool waiting = true;




int main()
{

  sysinit();

    // USART2->CR1 |= USART_CR1_TE;  // start transmit
    // sendString("\r\nSTM32F4DISCOVERY Booted up!\r\n");
    // sendString("===========================================\r\n");


    // const auto deviceId = DeviceId::get();
    // uint8_t deviceIdBytes[DeviceId::LENGTH];
    // char deviceIdHexString[2 * DeviceId::LENGTH + 1];
    // deviceId.get_bytes(&deviceIdBytes[0]);
    // stringio::hexlify(sizeof(deviceIdBytes), &deviceIdBytes[0], &deviceIdHexString[0]);


    // char buffer[256];
    // stringio::snprintf(buffer, sizeof(buffer), "Test(%%d for 0) == %d \r\n", 0); sendString(buffer);
    // stringio::snprintf(buffer, sizeof(buffer), "Test(%%d for 1) == %d \r\n", 1); sendString(buffer);
    // stringio::snprintf(buffer, sizeof(buffer), "Test(%%d for 10) == %d \r\n", 10); sendString(buffer);
    // stringio::snprintf(buffer, sizeof(buffer), "Test(%%d for 15) == %d \r\n", 15); sendString(buffer);
    // stringio::snprintf(buffer, sizeof(buffer), "Test(%%d for 101) == %d \r\n", 101); sendString(buffer);
    // stringio::snprintf(buffer, sizeof(buffer), "Test(%%d for 9876) == %d \r\n", 9876); sendString(buffer);
    // stringio::snprintf(buffer, sizeof(buffer), "Test(%%d for 123456) == %d \r\n", 123456); sendString(buffer);
    // stringio::snprintf(buffer, sizeof(buffer), "Test(%%x for 0xff) == 0x%x \r\n", 0xff); sendString(buffer);

    // bool redLedOn = false;

    uint32_t counter = 0;
    while (1)
    {

        // bool currentButtonHeldState = (GPIOA->IDR & 0x01) != 0;
        // if (currentButtonHeldState != lastButtonStateHeld)
        // {
        //     // Detected change in button state.
        //     // If the debounce cooldown is over, then handle the event.
        //     if (ticks > buttonCooldownExpiration)
        //     {

        //         if (currentButtonHeldState)
        //         {
        //             presses += 1;
        //         }

        //         // Save previous state to detect press AND release events.
        //         lastButtonStateHeld = currentButtonHeldState;

        //         // Prevent a bounce from re-triggering the event for 25 ms
        //         buttonCooldownExpiration = ticks + 25;
        //     }
        // }

        if (waiting)
        {
            // TODO: Wait for data ready signal from LIS3DSH?
            // Can this be ready via SPI, or do I have to use DRDY hardware line?
            continue;
        }

        // redLedOn = ! redLedOn;
        // if (redLedOn)
        // {
            GPIOD->BSRR = GPIO_BSRR_BS14;
        // }
        // else
        // {
            // GPIOD->BSRR = GPIO_BSRR_BR14;
        // }




        GPIOD->BSRR = GPIO_BSRR_BS14;
        // =====================================================================

        // Read info from LIS3DSH motion sensor
        // GPIOE->BSRR |= GPIO_BSRR_BR3;  // activate chip select

        // // First send the register to read.
        // uint8_t data =
        //     ((lis3dsh::WHO_AM_I & 0x7f) << 1) |  // register to read
        //     ((0x0001 & 0x1) << 0);  // read mode

        // SPI1->DR = data;

        // // wait for transmit and receive to finish
        // // (TODO: Figure out how to use interrupts)
        // while ((SPI1->SR & SPI_SR_TXE) == 0 &&
        //        (SPI1->SR & SPI_SR_RXNE) != 0);

        // uint8_t sensorData = SPI1->DR;

        // GPIOE->BSRR |= GPIO_BSRR_BS3;  // deactivate chip select
        // =====================================================================
        GPIOD->BSRR = GPIO_BSRR_BR14;


        // stringio::snprintf(buffer, sizeof(buffer), "Sensor Data: 0x%d  (Line %d) \r\n", sensorData, __LINE__);
        // sendString(buffer);





        counter += 1;


        waiting = true;
        // GPIOD->BSRR = GPIO_BSRR_BS14;

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

    // const bool useCirclePattern = presses % 2 == 0;
    if (TIM2->SR & TIM_SR_CC1IF)
    {
        // if (useCirclePattern)
        // {
        //     GPIOD->BSRR =
        //         GPIO_BSRR_BS12 |
        //         GPIO_BSRR_BR13 | GPIO_BSRR_BR14 | GPIO_BSRR_BR15;
        // }
        // else
        // {
        //     setLeds(true, true, false, false);
        // }
        TIM2->SR &= ~TIM_SR_CC1IF;
    }
    else if (TIM2->SR & TIM_SR_CC2IF)
    {
        GPIOD->BSRR = GPIO_BSRR_BS15;
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
        waiting = false;
        GPIOD->BSRR = GPIO_BSRR_BR15;
        // if (useCirclePattern)
        // {
        //     GPIOD->BSRR =
        //         GPIO_BSRR_BS15 |
        //         GPIO_BSRR_BR12 | GPIO_BSRR_BR13 | GPIO_BSRR_BR14;
        // }
        TIM2->SR &= ~TIM_SR_UIF;
    }
}
