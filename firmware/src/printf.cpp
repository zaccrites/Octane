
// https://github.com/mpaland/printf#usage

#include <stdint.h>
#include <stm32f4xx.h>


extern "C" void _putchar(char character)
{
    USART2->DR = static_cast<uint32_t>(character);
    while ( ! (USART2->SR & USART_SR_TXE));
}
