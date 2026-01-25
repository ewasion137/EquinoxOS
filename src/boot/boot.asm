[bits 16]
[org 0x7c00]

KERNEL_OFFSET equ 0x1000 

start:
    mov [BOOT_DRIVE], dl 

    ; 1. Загружаем ядро
    mov bx, KERNEL_OFFSET
    mov dh, 50          ; Увеличил до 50 секторов, ядро будет расти!
    mov dl, [BOOT_DRIVE]
    call disk_load

    ; --- 2. УМНАЯ НАСТРОЙКА VESA (800x600, 32 bit) ---
    mov ax, 0x4F01      ; Функция получения инфы о режиме
    mov cx, 0x4143       ; Код режима (0x115 = 800x600, 32-bit TrueColor)
    mov di, 0x7000      ; Сюда BIOS запишет структуру VBE Mode Info
    int 0x10

    ; Сохраняем адрес видеобуфера (он лежит по смещению 40 в структуре)
    mov eax, [0x7028]   
    mov [vbe_fb], eax   ; Сохраняем для ядра

    mov ax, 0x4F02      ; Функция установки режима
    mov bx, 0x4143      ; 0x4000 (флаг LFB) + 0x115 (режим)
    int 0x10
    ; -----------------------------------------------

    ; 3. Переходим в 32-битный режим
    cli                 
    lgdt [gdt_descriptor] 
    mov eax, cr0
    or eax, 0x1         
    mov cr0, eax
    jmp CODE_SEG:init_32bit 

[bits 32]
init_32bit:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov esp, 0x90000    
    
    ; ПЕРЕДАЕМ АДРЕС ЭКРАНА В ЯДРО ЧЕРЕЗ СТЕК
    push dword [vbe_fb]
    
    push dword [vbe_fb] ; Кладем адрес экрана в стек
    jmp KERNEL_OFFSET
    jmp $

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

gdt_start:
    dq 0x0          ; Пустой дескриптор (обязательно)

gdt_code:           ; Сегмент кода
    dw 0xffff       ; Limit (0-15)
    dw 0x0          ; Base (0-15)
    db 0x0          ; Base (16-23)
    db 10011010b    ; Access byte
    db 11001111b    ; Flags + Limit (16-19)
    db 0x0          ; Base (24-31)

gdt_data:           ; Сегмент данных
    dw 0xffff       ; Limit (0-15)
    dw 0x0          ; Base (0-15)
    db 0x0          ; Base (16-23)
    db 10010010b    ; Access byte
    db 11001111b    ; Flags + Limit (16-19)
    db 0x0          ; Base (24-31)

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
BOOT_DRIVE db 0
vbe_fb dd 0             ; Тут будет храниться адрес экрана

times 510-($-$$) db 0
dw 0xaa55