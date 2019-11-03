
#pragma once


// namespace stm32
// {


struct USART_TypeDef
{
    uint32_t SR;
    uint32_t DR;
    uint32_t BRR;
    uint32_t CR1;
    uint32_t CR2;
    uint32_t CR3;
    uint32_t GTPR;
};


constexpr uint32_t USART_SR_TXE = (1 << 7);
constexpr uint32_t USART_SR_TC = (1 << 6);
constexpr uint32_t USART_SR_RXNE = (1 << 5);

constexpr uint32_t USART_DR_MASK = 0x000000ff;

constexpr uint32_t USART_CR1_UE = (1 << 13);
constexpr uint32_t USART_CR1_TE = (1 << 3);
constexpr uint32_t USART_CR1_RE = (1 << 2);


constexpr uint32_t USART6_BASE = 0x40011400;  // APB2
constexpr uint32_t USART1_BASE = 0x40011000;  // APB2
constexpr uint32_t UART8_BASE  = 0x40007c00;  // APB1
constexpr uint32_t UART7_BASE  = 0x40007800;  // APB1
constexpr uint32_t UART5_BASE  = 0x40005000;  // APB1
constexpr uint32_t UART4_BASE  = 0x40004c00;  // APB1
constexpr uint32_t USART3_BASE = 0x40004800;  // APB1
constexpr uint32_t USART2_BASE = 0x40004400;  // APB1


auto* const USART6 = reinterpret_cast<volatile USART_TypeDef*>(USART6_BASE);
auto* const USART1 = reinterpret_cast<volatile USART_TypeDef*>(USART1_BASE);
auto* const UART8 = reinterpret_cast<volatile USART_TypeDef*>(UART8_BASE);
auto* const UART7 = reinterpret_cast<volatile USART_TypeDef*>(UART7_BASE);
auto* const UART5 = reinterpret_cast<volatile USART_TypeDef*>(UART5_BASE);
auto* const UART4 = reinterpret_cast<volatile USART_TypeDef*>(UART4_BASE);
auto* const USART3 = reinterpret_cast<volatile USART_TypeDef*>(USART3_BASE);
auto* const USART2 = reinterpret_cast<volatile USART_TypeDef*>(USART2_BASE);


// constexpr uint32_t CalculateUsartBRR(uint32_t baud, bool over8 = false)
// {
//     // Defaults to 9600 baud at 16 MHz
//     uint32_t brr = ;

//     // TODO: Calculate for real.
//     return 0;
// }


// }
