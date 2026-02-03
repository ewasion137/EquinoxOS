ENTRY(kmain)

SECTIONS {
    . = 0xffffffff80100000;  /* <-- САМОЕ ВАЖНОЕ ИЗМЕНЕНИЕ */

    .text : {
        /* Тут твои .text и .multiboot_header (если ты его еще используешь) */
        *(.text)
    }

    /* Остальные секции */
    .data ALIGN(0x1000) : { *(.data) }
    .rodata ALIGN(0x1000) : { *(.rodata) }
    .bss ALIGN(0x1000) : { *(.bss) }
}