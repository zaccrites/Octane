
#include "init.hpp"
#include <printf.h>

#include "fpga.hpp"


#include <stdint.h>
#include <stm32f4xx.h>




extern volatile bool fpgaLedOn;


extern volatile bool newSampleAvailable;
extern volatile uint16_t currentSample;


int main()
{
    GPIOD->BSRR = GPIO_BSRR_BS12;

    octane::init();
    printf("\r\n======================== OCTANE ======================== \r\n\r\n");

    printf("Starting FPGA initialization \r\n");
    octane::fpga::reset();
    // octane::fpga::init();

    // octane::fpga::setNoteOn(0, true);

    printf("Finished FPGA initialization \r\n");


    // bool fpgaLedOn = true;
    // octane::fpga::writeRegister(
    //     (0b10 << 14) | (0x12 << 8) | (0 << 5) | 0,
    //     fpgaLedOn ? 0x0001 : 0x0000
    // );


    SPI2->CR1 |= SPI_CR1_SPE;    // enable SPI before comms


    // int i = 0;
    bool fpgaLedOnLast = false;

    int counter = 0;
    while (true)
    {
        // printf("Hello World! %d \r\n", i++);

        if (newSampleAvailable)
        {
            counter += 1;
            if (counter < 1000) continue;
            else counter = 0;
        }

        // if (fpgaLedOn != fpgaLedOnLast)
        {
            fpgaLedOnLast = fpgaLedOn;

            // SPI2->CR1 |= SPI_CR1_SPE;    // enable SPI before comms
            octane::fpga::writeRegister(
                (0b10 << 14) | (0x12 << 8) | (0 << 5) | 0,
                // 0x0001,
                // fpgaLedOn ? 0x0001 : 0x0000,
                // fpgaLedOn ? 0xf731 : 0xf730

                fpgaLedOn ? 0xffff : 0xff0f
                // fpgaLedOn ? 0xfff0 : 0xff00
            );
            // SPI2->CR1 &= ~SPI_CR1_SPE;    // disable SPI to release NSS

        }

        // if (newSampleAvailable)
        // {
        //     printf("New sample: 0x%04x \r\n", currentSample);
        //     DAC1->DHR12R2 = currentSample >> 8;
        //     newSampleAvailable = false;
        // }

    }

}
