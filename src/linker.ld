/* linker ld */

ENTRY(kmain)

SECTIONS
{
    . = 0x1000;

    /* Сливаем всё в одну секцию text, чтобы ничего не потерялось */
    .text : ALIGN(4K)
    {
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata*)
        *(.rdata)
        *(.rdata*)
    }

    .data : ALIGN(4K)
    {
        *(.data)
        *(.data.*)
    }

    .bss : ALIGN(4K)
    {
        *(.COMMON)
        *(.bss)
        *(.bss.*)
    }
}