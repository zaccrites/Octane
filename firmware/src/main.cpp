
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

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOCEN;
    RCC->APB1ENR |=
        RCC_APB1ENR_TIM2EN |    // enable TIM2
        RCC_APB1ENR_TIM3EN |    // enable TIM3
        RCC_APB1ENR_USART2EN |  // enable USART2
        RCC_APB1ENR_SPI2EN |
        RCC_APB1ENR_DACEN;

    // RCC->APB2ENR |=
    //     RCC_APB2ENR_SPI1EN;     // enable SPI1



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


    // TIM3 will just trigger its interrupt at 46.875 kHz (though this comes to more like 46.516 kHz)
    TIM3->PSC = 0;
    TIM3->ARR = 171;  // If I clock the MCU timer higher, then I can get a more accurate sample clock
    TIM3->DIER = TIM_DIER_UIE;
    TIM3->CR1 = TIM_CR1_CEN;  // Start timer




    // Setup DAC2  (PA5)

    // Configure USART2
    // See manual of Figure 26 for GPIO alt function mapping
    GPIOA->MODER |=
        (0b11 << GPIO_MODER_MODER5_Pos);  // DAC2_OUT=PA5
    // GPIOA->AFR[0] |= ???;

    DAC1->DHR12R2 = 0;

    DAC1->CR = DAC_CR_EN2 | DAC_CR_BOFF2;
    // (0b111 << DAC_CR_TSEL2_Pos) |




    // Setup SPI2 to talk to FPGA
    // For now just bit-bang it
    // SPI2_SCK = PB13
    // SPI2_MISO = PB14
    // SPI2_MOSI = PB15
    //
    // TODO: Use alternate function for these pins so that the SPI hardware takes over
    GPIOB->MODER |=
        (GPIO_MODER_ALTERNATE << GPIO_MODER_MODER13_Pos) |  // SCK
        (GPIO_MODER_ALTERNATE << GPIO_MODER_MODER14_Pos) |  // MISO
        (GPIO_MODER_ALTERNATE << GPIO_MODER_MODER15_Pos) |  // MOSI
        (0b01 << GPIO_MODER_MODER8_Pos);  // FPGA_RESET is an output

    GPIOB->AFR[1] |=
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

    SPI2->CR1 =
        SPI_CR1_MSTR |  // act as SPI master
        (0b001 << SPI_CR1_BR_Pos) |  // set baud rate to f_PCLK / 4 (2 MHz)
        SPI_CR1_DFF; // |   // use 16 bit frame, MSB out first
        // SPI_CR1_SSM;    // software slave management


    SPI2->CR2 =
        SPI_CR2_FRF; // |  // SCK would not start until I added this

    //     // TODO: Frame format for Motorola vs TI?
        // SPI_CR2_TXEIE |  // trigger interrupt when TX buffer empty
        // SPI_CR2_RXNEIE;  // trigger interrupt when RX buffer not empty
    //     // TODO: Use TX and RX buffer DMA enable




    const uint32_t MAIN_CLOCK_FREQ = 8000000;
    SysTick_Config(MAIN_CLOCK_FREQ / 1000);  // 1ms ticks

    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_EnableIRQ(TIM3_IRQn);
    NVIC_EnableIRQ(SPI2_IRQn);

    __enable_irq();


    SPI2->CR1 |= SPI_CR1_SPE;    // enable SPI

}



// TODO: Make class for this
// typedef std::pair<std::uint16_t, std::uint16_t> RegWriteCmd;
// std::array<RegWriteCmd, 1024> regWriteCmdBuffer;
// std::size_t regWriteCmdBufferSize;

// bool queueRegWriteCmd()

// TODO: Create a critical section class to disable and re-enable interrupts
// (possibly specific interrupts) using RAII around e.g. modifying the command queue



volatile uint16_t currentSample;
volatile bool fpgaSpiReady = true;
extern "C" void SPI2_IRQHandler()
{
    if (SPI2->SR & SPI_SR_TXE)
    {
        fpgaSpiReady = true;
    }
    else if (SPI2->SR & SPI_SR_RXNE)
    {
        currentSample = SPI2->DR;
    }
}




// TODO: Use interrupt
// TODO: Move these to usart.cpp or something
void sendChar(char byte) {
    USART2->DR = static_cast<uint32_t>(byte);  // TODO: Add mask for 8 (?) bits
    // GPIOD->BSRR = GPIO_BSRR_BS12;
    while ( ! (USART2->SR & USART_SR_TXE));
    // GPIOD->BSRR = GPIO_BSRR_BR12;
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



// #include <cstring>



volatile int x;
volatile int y;
void wait()
{
    for (int i = 0; i < 16; i++)
    {
        y = i;
        if (x) break;
    }
}


// Do I need a timer to slow this down at all?
// 1 MHz on an 8 MHz main clock is basically just the overhead anyway
uint16_t fpgaSpiSend(uint16_t sendData)
{
    // TODO: Just use the real SPI peripheral instead.

    uint16_t recvData = 0;

    // Setup SPI2 to talk to FPGA
    // For now just bit-bang it
    // SPI2_SCK = PB13
    // SPI2_MISO = PB14
    // SPI2_MOSI = PB15

    char buffer[64];
    for (uint16_t i = 0; i < 16; i++)
    {
        // If this continues to not work, try using TIM3 to delay properly.
        // Otherwise use the scope to verify the MOSI and SCK signals are right.
        // If all else fails, try the SPI peripheral?


        // Actually what I should really do is add the RGB LED driver
        // as a writable register to the FPGA to verify that the registers
        // are actually getting written by the fpga logic.
        //
        // Make a simple project like the other one to test the LEDDA PWM module
        //

        // Alternatively, just tap off 8 or so pins from the FPGA IO which
        // I can also make writable via an SPI register. Heck, even a single
        // LED would be enough if setting up the PWM IP is too much trouble.
        // (though I wanted to do it anyway, so it's not much of a loss to do)


        GPIOB->BSRR = GPIO_BSRR_BR13;  // SCK falling edge
        // wait(); // sendString("Waiting after SCK falling edge \r\n");
        // wait?

        bool mosiBit = sendData & 0x8000;
        sendData = sendData << 1;
        GPIOB->BSRR = mosiBit ? GPIO_BSRR_BS15 : GPIO_BSRR_BR15;  // MOSI
        // snprintf(buffer, sizeof(buffer), "  Sending bit %d = %d \r\n", 16-i, mosiBit); sendString(buffer);
        // snprintf(buffer, sizeof(buffer), "%d ", mosiBit); sendString(buffer);
        // wait?

        GPIOB->BSRR = GPIO_BSRR_BS13;  // SCK rising edge
        // wait(); // sendString("Waiting after SCK rising edge \r\n");
        // wait?

        bool misoBit = GPIOB->IDR & GPIO_IDR_ID14;
        recvData = (recvData << 1) | misoBit;
        // snprintf(buffer, sizeof(buffer), "  Got bit %d = %d \r\n", 16-i, misoBit); sendString(buffer);
        // wait?
    }

    // sendString("\r\n");

    return recvData;
}




void writeFpgaRegister(uint16_t registerNumber, uint16_t value)
{
    // m_SPI_SendQueue.push(registerNumber);
    // m_SPI_SendQueue.push(value);
    fpgaSpiSend(registerNumber);
    fpgaSpiSend(value);
}

void writeOperatorRegister(uint8_t voiceNum, uint8_t operatorNum, uint8_t parameter, uint16_t value)
{
    const uint16_t registerNumber = (0b10 << 14) | (parameter << 8) | (operatorNum << 5) | voiceNum;
    writeFpgaRegister(registerNumber, value);
}



#define SINE_TABLE_IMPLEMENTATION
#include "sine_table.h"

#include <cmath>
void populateSineTable()
{
    const uint16_t TABLE_SIZE = 16 * 1024;
    const uint16_t BIT_DEPTH = 15;
    const uint16_t MAX_RANGE = 1 << BIT_DEPTH;
    const uint16_t MASK = MAX_RANGE - 1;

    for (uint16_t i = 0; i < TABLE_SIZE; i++)
    {
        // // https://zipcpu.com/dsp/2017/08/26/quarterwave.html
        // const double phase =
        //     2.0 * M_PI *
        //     (2.0 * static_cast<double>(i) + 1) /
        //     (2.0 * static_cast<double>(TABLE_SIZE) * 4.0);

        // // https://stackoverflow.com/a/12946226
        // const double sineValue = std::sin(phase);
        // const uint16_t value = static_cast<uint16_t>(sineValue * MAX_RANGE) & MASK;

        const uint16_t value = SINE_TABLE[i];
        const uint16_t registerNumber = (0b11 << 14) | i;
        writeFpgaRegister(registerNumber, value);
    }
}


// const uint16_t SAMPLE_FREQUENCY = 44100;
// const uint16_t SAMPLE_FREQUENCY = 96000;
const uint16_t SAMPLE_FREQUENCY = 46875;  // 12 MHz / 256 = 46.875 kHz


uint16_t phaseStepForFrequency(double frequency) {
    // Formula:
    // phaseStep = 2^N * f / FS
    // where N is the number of bits of phase accumulation
    // FS is the sample frequency
    // and f is the desired tone frequency
    return static_cast<uint16_t>(
        static_cast<double>(1 << 16) *
        frequency /
        static_cast<double>(SAMPLE_FREQUENCY)
    );
}




const uint8_t OP_PARAM_PHASE_STEP  { 0x00 };
const uint8_t OP_PARAM_ALGORITHM      { 0x01 };

const uint8_t OP_PARAM_ENVELOPE_ATTACK_LEVEL   { 0x02 };
const uint8_t OP_PARAM_ENVELOPE_SUSTAIN_LEVEL  { 0x03 };
const uint8_t OP_PARAM_ENVELOPE_ATTACK_RATE    { 0x04 };
const uint8_t OP_PARAM_ENVELOPE_DECAY_RATE     { 0x05 };
const uint8_t OP_PARAM_ENVELOPE_RELEASE_RATE   { 0x06 };

const uint8_t OP_PARAM_FEEDBACK_LEVEL   { 0x07 };


const uint8_t PARAM_NOTEON_BANK0  { 0x10 };
const uint8_t PARAM_NOTEON_BANK1  { 0x11 };
const uint8_t PARAM_LED_CONFIG  { 0x12 };



// TODO
static bool m_NoteOnState[32];

void clearNotesOn()
{
    for (auto& state : m_NoteOnState) state = false;

    writeOperatorRegister(0, 0, PARAM_NOTEON_BANK0, 0x0000);
    writeOperatorRegister(0, 0, PARAM_NOTEON_BANK1, 0x0000);
}

void setNoteOn(uint8_t voiceNum, bool noteOn)
{
    voiceNum = voiceNum % 32;
    if (m_NoteOnState[voiceNum] == noteOn)
    {
        return;
    }
    m_NoteOnState[voiceNum] = noteOn;

    const uint8_t bank = voiceNum / 16;
    uint16_t newRegisterValue = 0;
    for (int i = 0; i < 16; i++)
    {
        newRegisterValue |= static_cast<uint16_t>(m_NoteOnState[16 * bank + i]) << i;
    }

    if (bank == 0)
    {
        // TODO: This isn't really a "voice op" parameter anymore.
        // Need to reorganize registers now that I need all 14 bits
        // of a "global" address for the sine table.
        // If I can write the whole thing at once then this problem
        // probably goes away (could just have 8 bit addresses, honestly).
        writeOperatorRegister(0, 0, PARAM_NOTEON_BANK0, newRegisterValue);
    }
    else
    {
        writeOperatorRegister(0, 0, PARAM_NOTEON_BANK1, newRegisterValue);
    }
}


void configureFpgaRegisters()
{


    // Reset FPGA
    GPIOB->BSRR = GPIO_BSRR_BS8;
    // wait?
    GPIOB->BSRR = GPIO_BSRR_BR8;

    sendString("Starting SPI configuration \r\n");
    populateSineTable();


    // INVSTATE occurs on use of double


    std::array<uint16_t, 8> algorithmWords = {
        //       7654321
        //xxxxxx mmmmmmm nnn c
        0b000000'0000000'000'0,  // OP1
        0b000000'0000001'000'1,  // OP2
        0b000000'0000000'000'0,  // OP3
        0b000000'0000000'000'0,  // OP4
        0b000000'0000000'000'0,  // OP5
        0b000000'0000000'000'0,  // OP6
        0b000000'0000000'000'0,  // OP7
        0b000000'0000000'000'0,  // OP8
    };


    clearNotesOn();
    for (uint16_t voiceNum = 0; voiceNum < 32; voiceNum++)
    {
        double noteBaseFrequency;
        // if (voiceNum < 16)
        // {
        //     // noteBaseFrequency = 500.0;
        //     noteBaseFrequency = 350.0;
        // }
        // else
        {
            // noteBaseFrequency = 1000.0;
            noteBaseFrequency = 523.251;  // C5
        }
        // noteBaseFrequency = 100.0 * (1 + voiceNum);
        // noteBaseFrequency = 440.0;

        for (uint16_t opNum = 0; opNum < 8; opNum++)
        {

            uint8_t feedbackLevel = (opNum == 0) ? 255 : 0;
            writeOperatorRegister(voiceNum, opNum, OP_PARAM_FEEDBACK_LEVEL, feedbackLevel);

            writeOperatorRegister(voiceNum, opNum, OP_PARAM_ALGORITHM, algorithmWords[opNum]);

            // double frequencyRatio = 1.0;
            // uint16_t phaseStep = phaseStepForFrequency(noteBaseFrequency * frequencyRatio);
            uint16_t phaseStep = 600;
            writeOperatorRegister(voiceNum, opNum, OP_PARAM_PHASE_STEP, phaseStep);

            bool isCarrier = algorithmWords[opNum] & 0x0001;

            // TODO: Clean this all up. Put into a Patch class that can
            // do all these calculations and output real values to pass
            // to the microcontroller after loading the binary configuration.

            auto modulationIndexForLevel = [](uint16_t level) -> double {
                // Vary linearly up to I=15
                return level * 15.0 / 1000.0;
            };

            auto carrierAmplitudeForLevel = [](uint16_t level) -> double {
                // 1000 is 100% intensity, but 500 is half as loud (10% intensity).
                // 250 is a quarter as loud (1% intensity), etc.
                const double exponent = std::log(10) / std::log(2);
                return std::pow(level, exponent) / std::pow(1000.0, exponent);
            };


            auto fixOperatorLevel = [carrierAmplitudeForLevel, modulationIndexForLevel, phaseStep, isCarrier](uint16_t level) -> uint16_t {
                if (isCarrier)
                {
                    double multiplier = carrierAmplitudeForLevel(level);
                    // printf("multiplier for level %d is %f \n", level, multiplier);
                    return static_cast<uint16_t>(0x3fff * multiplier);
                }
                else
                {
                    double modulationIndex = modulationIndexForLevel(level);
                    return static_cast<uint16_t>(modulationIndex * phaseStep);
                }
            };
            auto fixOperatorRate = [carrierAmplitudeForLevel, modulationIndexForLevel, phaseStep, isCarrier](uint16_t rate) -> uint16_t {

                if (isCarrier)
                {
                    double multiplier = carrierAmplitudeForLevel(rate);
                    return static_cast<uint16_t>(0x0fff * multiplier);
                }
                else
                {
                    double modulationIndex = modulationIndexForLevel(rate);
                    return static_cast<uint16_t>(modulationIndex * phaseStep) / 4;
                }
            };

            uint16_t attackLevel = 1000;
            uint16_t sustainLevel = 1000;
            uint16_t attackRate = 1000;
            uint16_t decayRate = 1000;
            uint16_t releaseRate = 1000;

            writeOperatorRegister(voiceNum, opNum, OP_PARAM_ENVELOPE_ATTACK_LEVEL, 0xffff); //fixOperatorLevel(attackLevel));
            writeOperatorRegister(voiceNum, opNum, OP_PARAM_ENVELOPE_SUSTAIN_LEVEL, 0xffff); //fixOperatorLevel(sustainLevel));
            writeOperatorRegister(voiceNum, opNum, OP_PARAM_ENVELOPE_ATTACK_RATE, 0xffff); // fixOperatorRate(attackRate));
            writeOperatorRegister(voiceNum, opNum, OP_PARAM_ENVELOPE_DECAY_RATE, 0xffff); // fixOperatorRate(decayRate));
            writeOperatorRegister(voiceNum, opNum, OP_PARAM_ENVELOPE_RELEASE_RATE, 0xffff); // fixOperatorRate(releaseRate));

        }
    }

    for (uint16_t voiceNum = 0; voiceNum < 32; voiceNum++)
    {
        if (voiceNum == 0)
        {
            setNoteOn(voiceNum, true);
        }
    }

    sendString("Finished SPI configuration \r\n");

}




volatile bool fpgaLedOn = false;

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


    // GPIOB->BSRR = GPIO_BSRR_BR13;  // SCK falling edge
    // GPIOB->BSRR = GPIO_BSRR_BS13;  // SCK rising edge



    // configureFpgaRegisters();



    SPI2->DR = (0b10 << 14) | (0x12 << 8) | (0 << 5) | 0;



    // writeOperatorRegister(0, 0, PARAM_LED_CONFIG, 0b111);

    bool first = true;
    bool lastLedState = false;
    while (1)
    {
        // uint16_t sample = fpgaSpiSend(0x0000);

        // snprintf(buffer, sizeof(buffer), "Read 0x%04x from FPGA \r\n", sample);
        // sendString(buffer);

        // DAC1->DHR12R2 = sample & 0x0fff;
        // wait?


        // // Start the first SPI transfer
        // GPIOD->BSRR = GPIO_BSRR_BS12;
        // SPI2->DR = (0b10 << 14) | (0x12 << 8) | (0 << 5) | 0;
        // while ( ! (SPI2->SR & SPI_SR_TXE));  // wait

        // GPIOD->BSRR = GPIO_BSRR_BR12;
        // SPI2->DR = 0b0000'0000'0000'0001;
        // while ( ! (SPI2->SR & SPI_SR_TXE));  // wait


        // if (first || lastLedState != fpgaLedOn)
        {


            lastLedState = fpgaLedOn;

            GPIOD->BSRR = GPIO_BSRR_BS12 | GPIO_BSRR_BR14;  // GREEN ON
            while ( ! (SPI2->SR & SPI_SR_TXE));
            SPI2->DR = (0b10 << 14) | (0x12 << 8) | (0 << 5) | 0;
            // fpgaSpiReady = false;

            GPIOD->BSRR = GPIO_BSRR_BR12 | GPIO_BSRR_BS14;  // RED ON
            while ( ! (SPI2->SR & SPI_SR_TXE));
            SPI2->DR = fpgaLedOn ? 0x0001 : 0x0000;
            // fpgaSpiReady = false;
        }

        first = false;
    }

}




extern "C" void TIM3_IRQHandler()
{
    if (TIM3->SR & TIM_SR_UIF)
    {
        static bool state = false;
        state = ! state;

        if (state)
            DAC1->DHR12R2 = 0xfff / 2;
        else
            DAC1->DHR12R2 = 0x000;

        TIM3->SR &= ~TIM_SR_UIF;
    }
}



// GREEN LED = PD12
// ORANGE LED = PD13
// RED LED = PD14
// BLUE LED = PD15

extern "C" void TIM2_IRQHandler(void)
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
