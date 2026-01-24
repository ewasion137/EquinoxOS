[bits 32]
[global isr_stub]
[extern outb] ; Мы будем вызывать Си-шную функцию вывода в порт

isr_stub:
    pusha
    
    ; Сообщаем PIC, что прерывание обработано (EOI - End of Interrupt)
    ; Нужно послать 0x20 в порт 0x20
    push 0x20 ; значение
    push 0x20 ; порт
    call outb
    add esp, 8 ; чистим стек после вызова функции
    
    popa
    iret