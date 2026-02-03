; src/system/gdt_flush.asm
[bits 64]
global gdt_flush

gdt_flush:
    lgdt [rdi]        ; RDI - это адрес gdt_ptr

    ; Обнуляем сегментные регистры (MOV SS, 0 - запрещен)
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax        ; Оставляем как есть, но знаем, что это может быть проблемой

    ; 64-битный Far Jump
    push 0x08         ; Селектор кода (GDT entry 1)
    push ret_after_far_jump
    lretq

ret_after_far_jump:
    ret