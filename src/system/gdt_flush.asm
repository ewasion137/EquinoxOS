; src/system/gdt_flush.asm
[bits 64]
global gdt_flush

; gdt_flush вызывается из C, аргумент GDT_PTR в RDI
gdt_flush:
    lgdt [rdi]        

    ; Обнуляем сегментные регистры (обязательно в x64 Long Mode)
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; 64-битный Far Jump:
    ; Сначала кладем 64-битный адрес, куда прыгать, потом 16-битный селектор
    
    ; 1. Кладем адрес возврата (64-бит)
    lea rax, [ret_after_far_jump] 
    push rax
    
    ; 2. Кладем селектор кода (16-бит)
    push 0x08 

    ; 3. Выполняем 64-битный Far Return (аналог Far Jump в Long Mode)
    lretq
    
ret_after_far_jump:
    ret