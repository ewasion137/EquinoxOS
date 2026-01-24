[bits 32]
[global gdt_flush]

gdt_flush:
    mov eax, [esp+4]  ; Получаем адрес gdt_ptr
    lgdt [eax]        ; Загружаем новую GDT

    mov ax, 0x10      ; 0x10 — это смещение сегмента данных (2-й вход в GDT)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush   ; 0x08 — это смещение сегмента кода. Дальний прыжок.
.flush:
    ret