

#include <stm32f4xx.h>

// TODO: Wrap in a class which abstracts the output USART/SPI/whatever?
#include <printf.h>


#include "DeviceId.hpp"


void main()
{
    printf("Hello World! \r\n");

    auto deviceId = octane::DeviceId::get();
    printf("Device ID: %08x-%08x-%08x \r\n", deviceId.getPart1(), deviceId.getPart2(), deviceId.getPart3());

    bool ready = true;
    while (true)
    {

    }
}
