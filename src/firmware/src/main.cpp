

#include <stm32f4xx.h>

// TODO: Wrap in a class which abstracts the output USART/SPI/whatever?
#include <printf.h>


#include "DeviceId.hpp"
#include "SpiFlash.hpp"


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



    std::uint8_t buffer[32];
    printf("reading some flash... ");
    flash.readBytes(buffer, 0x0123, 32);
    // while (flash.readInProgress());  // wait
    printf("done! \r\n\r\n");

    printf("Flash Contents: \r\n");
    for (int i = 0; i < 32; i++)
    {
        printf("%02x ", buffer[i]);
        if ((i+1) % 8 == 0) printf("\r\n");
    }

    while (true)
    {

    }
}
