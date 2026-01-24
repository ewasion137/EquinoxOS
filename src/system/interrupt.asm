[bits 32]

; ГОВОРИМ, ЧТО ЭТИ ФУНКЦИИ ВИДНЫ СНАРУЖИ
[global isr_stub]
[global keyboard_handler]

[extern keyboard_callback]

; 1. Общий обработчик (заглушка)
isr_stub:
    pusha
    mov al, 0x20
    out 0x20, al
    popa
    iret

; 2. Обработчик клавиатуры
keyboard_handler:
    pusha
    call keyboard_callback
    mov al, 0x20
    out 0x20, al
    popa
    iret