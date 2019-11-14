
#include "init.hpp"
#include <printf.h>

#include "fpga.hpp"


#include <stdint.h>
#include <stm32f4xx.h>




extern volatile bool fpgaLedOn;


extern volatile bool newSampleAvailable;
extern volatile uint16_t currentSample;


void main()
{
    // GPIOD->BSRR = GPIO_BSRR_BS12;

    // octane::init();
    printf("\r\n======================== OCTANE ======================== \r\n\r\n");

    printf("Starting FPGA initialization \r\n");
    octane::fpga::reset();
    // octane::fpga::init();

    // octane::fpga::setNoteOn(0, true);

    printf("Finished FPGA initialization \r\n");


    SPI2->CR1 |= SPI_CR1_SPE;    // enable SPI before comms

    // int i = 0;
    bool fpgaLedOnLast = false;

    int counter = 0;
    while (true)
    {

        if (newSampleAvailable)
        {
            counter += 1;
            if (counter < 1000) continue;
            else counter = 0;
        }

        if (fpgaLedOn != fpgaLedOnLast)
        {
            fpgaLedOnLast = fpgaLedOn;

            GPIOD->BSRR = fpgaLedOn ? GPIO_BSRR_BS13 : GPIO_BSRR_BR13;


            octane::Fpga::getInstance().writeRegister(
                (0b10 << 14) | (0x12 << 8) | (0 << 5) | 0,
                fpgaLedOn ? 0xffff : 0xff0f
            );
            octane::Fpga::getInstance().commitRegisterWrites();


            printf("Current sample: 0x%04x \r\n", octane::Fpga::getInstance().getLatestSample());

        }

        // if (newSampleAvailable)
        // {
        //     printf("New sample: 0x%04x \r\n", currentSample);
        //     DAC1->DHR12R2 = currentSample >> 8;
        //     newSampleAvailable = false;
        // }

    }

}
