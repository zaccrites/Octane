
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 128K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 32K
}


/* Genereate a link error if heap and stack don't fit into RAM */
MIN_HEAP_SIZE = 4K;
MIN_STACK_SIZE = 16K;



SECTIONS
{
    /* Interrupt and reset vector table goes first */
    .isr_vector_table :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector_table))
        . = ALIGN(4);
    } > FLASH

    /* Program code */
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    .init_array :
    {
        . = ALIGN(4);
        _sinit_array = .;
        KEEP(*(SORT(.preinit_array)))
        KEEP(*(SORT(.preinit_array*)))
        KEEP(*(SORT(.init_array)))
        KEEP(*(SORT(.init_array*)))
        KEEP(*(SORT(.ctors)))
        KEEP(*(SORT(.ctors*)))
        . = ALIGN(4);
        _einit_array = .;
    } > FLASH

    /*
        .data init data is programmed into FLASH (the LMA address)
        and copied to RAM (the VMA address) at boot time.
    */
    _sidata = .;
    .data : AT(_sidata)
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM

    /*
        "Uninitialized" data section which gets zeroed-out at boot time.
    */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /*
        User heap and stack section (used to check that there is enough RAM left)
    */
    .user_heap_stack :
    {
        . = ALIGN(8);
        PROVIDE(end = .);
        PROVIDE(_end = .);
        KEEP(*(.heap))
        . = . + MIN_HEAP_SIZE;
        . = . + MIN_STACK_SIZE;
        PROVIDE(_stack_start = .);
        . = ALIGN(8);
    } > RAM

}

