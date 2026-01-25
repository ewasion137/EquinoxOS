[bits 32]

; ГОВОРИМ, ЧТО ЭТИ ФУНКЦИИ ВИДНЫ СНАРУЖИ
[global isr_stub]
[global keyboard_handler]
[global timer_handler]
[extern timer_callback]
[extern keyboard_callback]
[global irq12]
[extern mouse_handler]

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


timer_handler:
    pusha
    call timer_callback
    mov al, 0x20
    out 0x20, al
    popa
    iret
    
irq12:
    pushad
    call mouse_handler
    popad
    iretd