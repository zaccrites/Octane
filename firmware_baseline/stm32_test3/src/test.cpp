
#include "gpio.hpp"


bool blahblahblah()
{

    bool test = GpioA::checkInput(0);

    // uint32_t x = GpioA::raw()->IDR;

    return test;
}
