/* linker.ld */
OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(kmain)

PHDRS {
    text    PT_LOAD FLAGS(5);   /* R+X */
    rodata  PT_LOAD FLAGS(4);   /* R   */
    data    PT_LOAD FLAGS(6);   /* R+W */
}

SECTIONS {
    . = 0xffffffff80100000;

    /* ---- Код ---- */
    .text : {
        *(.text._start)
        *(.text)
        *(.text.*)          /* <-- критично с -mcmodel=large */
    } :text

    /* ---- Только чтение ---- */
    . = ALIGN(0x1000);
    .rodata : {
        *(.rodata)
        *(.rodata.*)

        /* Limine ОБЯЗАТЕЛЬНО должен найти эти маркеры в rodata */
        KEEP(*(.limine_requests_start_marker))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end_marker))
    } :rodata

    /* ---- Данные ---- */
    . = ALIGN(0x1000);
    .data : {
        *(.data)
        *(.data.*)
    } :data

    /* ---- BSS (нулевая инициализация) ---- */
    . = ALIGN(0x1000);
    .bss : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    } :data

    /* Выбрасываем мусор явно */
    /DISCARD/ : {
        *(.eh_frame)
        *(.note .note.*)
    }
}