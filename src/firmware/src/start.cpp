
#include <cstdint>

#include "init.hpp"


void main(void);


// These symbols defined by the linker aren't addresses themselves,
// but represent actual words in memory. You have to take their
// addresses in C++ before treating them like pointers.

extern std::uint32_t _sidata;
extern std::uint32_t _sdata;
extern std::uint32_t _edata;

extern std::uint32_t _sbss;
extern std::uint32_t _ebss;



// https://wiki.osdev.org/Calling_Global_Constructors#ARM_.28BPABI.29

extern void (*_sinit_array[1])();
extern void (*_einit_array[1])();

static void InitStaticConstructors()
{
    // Follow list of init functions, calling all.
    for (auto pInitFunc = _sinit_array; pInitFunc != _einit_array; pInitFunc++)
    {
        (*pInitFunc)();
    }
}


static void InitBssSection()
{
    // .bss section is aligned to 4 bytes, so we can init four bytes at a time
    std::uint32_t* pDest = &_sbss;
    while (pDest != &_ebss)
    {
        *pDest++ = 0x00000000;
    }
}


static void InitDataSection()
{
    // .data section is aligned to 4 bytes, so we can init four bytes at a time
    std::uint32_t* pSrc = &_sidata;
    std::uint32_t* pDest = &_sdata;
    while (pDest != &_edata)
    {
        *pDest++ = *pSrc++;
    }
}


extern "C" void _start(void)
{
    InitBssSection();
    InitDataSection();

    // TODO: Init hardware peripherals before even calling static
    // constructors? Could move init.cpp into this file.
    // This is important because initialization calls like pulling NSS
    // low in the Fpga static constructor
    octane::init();

    InitStaticConstructors();

    main();
}

