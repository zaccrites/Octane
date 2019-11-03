
#include <stdint.h>
#include <stddef.h>


void main(void);


// These symbols defined by the linker aren't addresses themselves,
// but represent actual words in memory. You have to take their
// addresses in C++ before treating them like pointers.

extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;

extern uint32_t _sbss;
extern uint32_t _ebss;



// An embedded system like this never exits, so destructors should never
// run anyway.

// https://wiki.osdev.org/C%2B%2B#GCC

void* __dso_handle = nullptr;

extern "C" int __aeabi_atexit(void *(destructor)(void*), void *arg, void* dso)
{
    // Unused parameters
    destructor = destructor;
    arg = arg;
    dso = dso;

    return 0;
}





// https://wiki.osdev.org/Calling_Global_Constructors#ARM_.28BPABI.29

extern void (*_sinit_array[1])();
extern void (*_einit_array[1])();

void InitStaticConstructors()
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
    uint32_t* pDest = &_sbss;
    while (pDest != &_ebss)
    {
        *pDest++ = 0x00000000;
    }
}


static void InitDataSection()
{
    // .data section is aligned to 4 bytes, so we can init four bytes at a time
    uint32_t* pSrc = &_sidata;
    uint32_t* pDest = &_sdata;
    while (pDest != &_edata)
    {
        *pDest++ = *pSrc++;
    }
}


extern "C" void _start(void)
{
    InitBssSection();
    InitDataSection();
    InitStaticConstructors();

    // TODO: Call .ctor functions, libc_init_arrays, etc.
    // https://stackoverflow.com/questions/15265295/understanding-the-libc-init-array

    main();
}
