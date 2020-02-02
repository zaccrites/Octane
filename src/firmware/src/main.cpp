

#include <stm32f4xx.h>

// TODO: Wrap in a class which abstracts the output USART/SPI/whatever?
#include <printf.h>


#include "DeviceId.hpp"
#include "SpiFlash.hpp"


// TODO: Use a function or class as a timer
namespace octane::isr {
    extern volatile std::uint32_t ticks;
}


void waitForMs(std::uint32_t delay)
{
    std::uint32_t waitUntil { octane::isr::ticks + delay };
    while (octane::isr::ticks < waitUntil);
}


void main()
{
    printf("Hello World! \r\n");

    auto deviceId = octane::DeviceId::get();
    printf("Device ID: %08x-%08x-%08x \r\n", deviceId.getPart1(), deviceId.getPart2(), deviceId.getPart3());


    // NOTE: Don't try to drive SPI lines unless
    // the FPGA is held in /CRESET! Wait to take control
    // of the bus until after the FPGA has configured itself
    // from the SPI flash.
    octane::Spi spi1(SPI1);
    octane::SpiFlash flash(spi1);

    // flash.setWriteEnable(true);

    // printf("erasing flash \r\n");
    // flash.eraseChip();

    // waitForMs(10);  // SPI flash needs 100 us to power up

    flash.jedecReadId();


    // std::uint8_t buffer[32];


    // // printf("reading some flash... ");
    // // flash.readBytes(buffer, 0x000000, sizeof(buffer));
    // // // while (flash.readInProgress());  // wait
    // // printf("done! \r\n\r\n");

    // // printf("Flash Contents: \r\n");
    // // for (int i = 0; i < 32; i++)
    // // {
    // //     printf("%02x ", buffer[i]);
    // //     if ((i+1) % 8 == 0) printf("\r\n");
    // // }

    // printf("writing some flash... ");
    // for (int i = 0; i < sizeof(buffer); i++)
    // {
    //     buffer[i] = i;
    // }
    // flash.writeBytes(buffer, 0x000000, sizeof(buffer));
    // printf("done! \r\n\r\n");

    // for (int i = 0; i < sizeof(buffer); i++)
    // {
    //     buffer[i] = 0xcc;
    // }
    // printf("reading some flash... ");
    // flash.readBytes(buffer, 0x000000, sizeof(buffer));
    // // while (flash.readInProgress());  // wait
    // printf("done! \r\n\r\n");

    // printf("Flash Contents: \r\n");
    // for (int i = 0; i < 32; i++)
    // {
    //     printf("%02x ", buffer[i]);
    //     if ((i+1) % 8 == 0) printf("\r\n");
    // }








    std::uint32_t counter = 0;
    while (true)
    {
        counter = (counter + 1) % 4;
        if      (counter == 0) GPIOC->BSRR = GPIO_BSRR_BS0 | GPIO_BSRR_BR1 | GPIO_BSRR_BR2 | GPIO_BSRR_BR3;
        else if (counter == 1) GPIOC->BSRR = GPIO_BSRR_BR0 | GPIO_BSRR_BS1 | GPIO_BSRR_BR2 | GPIO_BSRR_BR3;
        else if (counter == 2) GPIOC->BSRR = GPIO_BSRR_BR0 | GPIO_BSRR_BR1 | GPIO_BSRR_BS2 | GPIO_BSRR_BR3;
        else                   GPIOC->BSRR = GPIO_BSRR_BR0 | GPIO_BSRR_BR1 | GPIO_BSRR_BR2 | GPIO_BSRR_BS3;

        waitForMs(250);
    }
}
