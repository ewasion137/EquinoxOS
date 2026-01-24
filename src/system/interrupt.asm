[bits 32]
[global idt_load]

idt_load:
    mov eax, [esp+4]
    lidt [eax]
    ret

; Заглушка для прерывания (чтобы проц не паниковал)
[global isr_stub]
isr_stub:
    pusha       ; Сохраняем все регистры
    ; Тут позже будет вызов Си-кода
    popa        ; Восстанавливаем регистры
    iret        ; Возврат из прерывания (важно!)