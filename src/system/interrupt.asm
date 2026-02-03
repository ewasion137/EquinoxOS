; src/system/interrupt.asm
[bits 64]
[global isr_stub]
[global keyboard_handler]
[global timer_handler]
[extern timer_callback]
[extern keyboard_callback]

%macro SAVE_REGS 0
    push rax
    push rbx
    push rcx
    push rdx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    ; ... и остальные, но для IRQ этого хватит ...
%endmacro

%macro RESTORE_REGS 0
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

; 1. Общий обработчик (заглушка)
isr_stub:
    SAVE_REGS
    ; Тут должен быть вызов C-обработчика, но пока просто EOI
    mov al, 0x20
    out 0x20, al
    RESTORE_REGS
    iretq ; 64-битный возврат из прерывания

; 2. Обработчик клавиатуры
keyboard_handler:
    SAVE_REGS
    ; Вызов C-функции
    call keyboard_callback
    ; EOI - Конец прерывания
    mov al, 0x20
    out 0x20, al
    RESTORE_REGS
    iretq 

timer_handler:
    SAVE_REGS
    call timer_callback
    mov al, 0x20
    out 0x20, al
    RESTORE_REGS
    iretq