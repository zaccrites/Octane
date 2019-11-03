
#include "stm32f407.hpp"

#include "usart.hpp"
#include "deviceid.hpp"
#include "stringio.hpp"

#define ISR  extern "C" void

volatile uint32_t presses = 2;


// TODO: A version of the ISR macro that verified that the
// function name actually matched a real ISR name would be useful.
// I spent ~30 minutes trying to figure out why my "SysTick_IRQHandler"
// wasn't firing only to discover that it should be called "SysTick_Handler".


// A 32-bit counter allows for about 50 days of millisecond counting.
// A 64-bit counter would allow for 213 billion days of millisecond counting.
volatile uint32_t ticks = 2;
ISR SysTick_Handler()
{
    ticks += 1;

    GPIOD->BSRR = (ticks % 2 == 0) ? GPIO_BSRR_BS0 : GPIO_BSRR_BR0;

    // if (ticks % 250 == 0)
    // {
    //     presses += 1;
    // }
}




// TODO:
// 1. UART debugging output
// 2. Read out unique chip ID via UART
// 3. Read out motion sensor (I2C) via UART
// 4. Use motion sensor data and/or UART input to play sound via DAC chip




// Green LED: PD12
// Orange LED: PD13
// Red LED: PD14
// Blue LED: PD15


uint32_t counterValue = 3330;

volatile bool updateTimerValues = false;


bool setGpioE = false;

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


    // Configure USART2
    GPIOA->MODER |=
        (GPIO_MODER_ALTERNATE << 4) |  // TX=PA2
        (GPIO_MODER_ALTERNATE << 6);   // RX=PA3
    GPIOA->AFRL |=
        (7 << GPIO_AFRL2) |
        (7 << GPIO_AFRL3);



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


    // Increment a counter every millisecond
    SysTick_Config(8000);

    NVIC_EnableIRQ(TIM2_IRQn);
    __enable_irq();
}




// static bool buttonHeldSteadyState = false;
// static uint32_t buttonPressDebounceTimer;
// constexpr uint32_t BUTTON_DEBOUNCE_DISABLED = 0xffffffff;


// static bool lastButtonStateHeld = false;
// static uint32_t buttonCooldownExpiration = 0;



uint32_t factorial(uint32_t n)
{
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}


class TestInitClass
{
public:

    TestInitClass(uint32_t n) : m_Value {factorial(n)}
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
        GPIOE->MODER |= (0x01 << 20);
        GPIOE->BSRR = GPIO_BSRR_BS10;
    }

    ~TestInitClass()
    {
        GPIOE->BSRR = GPIO_BSRR_BR10;
    }

    uint32_t m_Value;

};


TestInitClass testInitClass {3};




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





volatile bool waiting = true;


void main(void)
{
    sysinit();

    const auto deviceId = DeviceId::get();
    uint8_t deviceIdBytes[DeviceId::LENGTH];
    char deviceIdHexString[2 * DeviceId::LENGTH + 1];
    deviceId.get_bytes(&deviceIdBytes[0]);
    stringio::hexlify(sizeof(deviceIdBytes), &deviceIdBytes[0], &deviceIdHexString[0]);


    char buffer[256];


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
            continue;
        }
        GPIOD->BSRR = GPIO_BSRR_BR14;


        USART2->CR1 |= USART_CR1_TE;  // start transmit

        counter += 1;
        stringio::snprintf(&buffer[0], sizeof(buffer), "Message %d %x %X %%:  DeviceID = ", counter, counter, counter);
        sendString(buffer);
        // sendString("Device ID: ");
        sendString(deviceIdHexString);
        sendString("\r\n");

        waiting = true;
        GPIOD->BSRR = GPIO_BSRR_BS14;

    }

}



ISR TIM2_IRQHandler(void)
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
