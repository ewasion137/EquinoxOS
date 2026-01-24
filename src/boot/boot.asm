[bits 16]
[org 0x7c00]

KERNEL_OFFSET equ 0x1000 ; Адрес, куда мы загрузим ядро

start:
    mov [BOOT_DRIVE], dl ; Сохраняем номер диска, с которого загрузились

    ; 1. Загружаем ядро с диска в память
    mov bx, KERNEL_OFFSET
    mov dh, 15          ; Сколько секторов грузим (пока 15 хватит)
    mov dl, [BOOT_DRIVE]
    call disk_load

    ; 2. Переходим в 32-битный режим
    cli                 ; Отключаем прерывания
    lgdt [gdt_descriptor] ; Загружаем таблицу GDT
    mov eax, cr0
    or eax, 0x1         ; Включаем бит защищенного режима
    mov cr0, eax
    jmp CODE_SEG:init_32bit ; Дальний прыжок для очистки конвейера

[bits 32]
init_32bit:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov esp, 0x90000    ; Устанавливаем указатель стека в безопасное место
    
    call KERNEL_OFFSET  ; Теперь тут будет лежать наш kernel_entry
    jmp $

; Вспомогательные функции
disk_load:
    push dx
    mov ah, 0x02
    mov al, dh
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02
    int 0x13
    pop dx
    ret

; GDT (Таблица дескрипторов сегментов)
gdt_start:
    dq 0x0
gdt_code:
    dw 0xffff, 0x0
    db 0x0, 10011010b, 11001111b, 0x0
gdt_data:
    dw 0xffff, 0x0
    db 0x0, 10010010b, 11001111b, 0x0
gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
BOOT_DRIVE db 0

times 510-($-$$) db 0
dw 0xaa55