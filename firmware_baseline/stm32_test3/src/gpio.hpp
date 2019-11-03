
#if !defined(GPIO_HPP)
#define GPIO_HPP

// #include <type_traits>
// https://stackoverflow.com/questions/15200516/compare-typedef-is-same-type
// Could implement is_same by myself


// TODO: Move to utils/fakestdlib folder of some kind

template<typename T, typename U>
struct is_same
{
    static const bool value = false;
};

// Specialization where T and U are the same
template<typename T>
struct is_same<T, T>
{
    static const bool value = true;
};



#include <stdint.h>


struct GPIO_TypeDef
{
    uint32_t MODER;
    uint32_t OTYPER;
    uint32_t OSPEEDR;
    uint32_t PUPDR;        /// Enable or disable internal pull ups on GPIO pins
    uint32_t IDR;
    uint32_t ODR;
    uint32_t BSRR;
    uint32_t LCKR;
    uint32_t AFR[2];
};

constexpr uint32_t GPIOD_BASE = 0x40020c00;
constexpr uint32_t GPIOA_BASE = 0x40020000;

// constexpr auto* GPIOA = reinterpret_cast<volatile GPIO_TypeDef*>(0x40020000);
// constexpr auto* GPIOB = reinterpret_cast<volatile GPIO_TypeDef*>(0x40020400);
// constexpr auto* GPIOC = reinterpret_cast<volatile GPIO_TypeDef*>(0x40020800);
// constexpr auto* GPIOD = reinterpret_cast<volatile GPIO_TypeDef*>(0x40020c00);
// constexpr auto* GPIOE = reinterpret_cast<volatile GPIO_TypeDef*>(0x40021000);
// constexpr auto* GPIOF = reinterpret_cast<volatile GPIO_TypeDef*>(0x40021400);
// constexpr auto* GPIOG = reinterpret_cast<volatile GPIO_TypeDef*>(0x40021800);
// constexpr auto* GPIOH = reinterpret_cast<volatile GPIO_TypeDef*>(0x40021c00);
// constexpr auto* GPIOI = reinterpret_cast<volatile GPIO_TypeDef*>(0x40022000);
// constexpr auto* GPIOJ = reinterpret_cast<volatile GPIO_TypeDef*>(0x40022400);
// constexpr auto* GPIOK = reinterpret_cast<volatile GPIO_TypeDef*>(0x40022800);

// constexpr volatile auto* GPIOD = new (0x40020c00) GPIO_TypeDef;




template<uint32_t GPIO_BASE>
class Gpio
{
    // Might have to get creative since I don't want
    // static_assert(is_same<GPIO_TypeDef*, GPIO_T>::value, "blah");


public:

    // static volatile * const GPIO_TypeDef = reinterpret_cast<

    // static const auto raw_address = GPIO_BASE;
    // static const auto* raw = reinterpret_cast<volatile GPIO_TypeDef*>(GPIO_BASE);

    static volatile GPIO_TypeDef* raw()
    {
        return reinterpret_cast<volatile GPIO_TypeDef*>(GPIO_BASE);
    }

    static bool checkInput(uint32_t index)
    {
        return (raw()->IDR & (1 << index)) != 0;
    }


};



typedef Gpio<GPIOA_BASE> GpioA;

#endif
