ENTRY(kmain)
SECTIONS
{
    . = 0x1000;
    .text : { *(.text) }
    .rodata : { *(.rodata*) }
    .rdata : { *(.rdata*) }
    .data : { *(.data) }
    .bss  : { *(.bss) }
}