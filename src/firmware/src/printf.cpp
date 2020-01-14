
// https://github.com/mpaland/printf#usage

// TODO: Make the output switchable?
// Having an "fprintf" that could point to different output streams
// would be convenient.

#include <cstdint>
#include <stm32f4xx.h>

extern "C" void _putchar(char character)
{
    // TODO: Non blocking?
    USART1->DR = static_cast<std::uint32_t>(character);
    while ( ! (USART1->SR & USART_SR_TXE));
}
