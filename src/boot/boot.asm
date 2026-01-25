[bits 16]
[org 0x7c00]

KERNEL_OFFSET equ 0x1000 

start:
    mov [BOOT_DRIVE], dl
    mov bp, 0x9000
    mov sp, bp

    ; --- 1. ЗАГРУЗКА ЯДРА (ТРИ ЧАСТИ) ---
    
    ; Часть 1: Читаем первые 127 секторов
    mov bx, KERNEL_OFFSET
    mov dh, 127
    mov dl, [BOOT_DRIVE]
    mov cl, 2
    call disk_load

    ; Часть 2: Читаем следующие 127 секторов
    add bx, 0xFE00      ; Сдвигаем адрес записи (127 * 512 = 0xFE00)
    mov dh, 127
    mov dl, [BOOT_DRIVE]
    mov cl, 129         ; Начинаем со 129-го сектора (2 + 127)
    call disk_load

    ; Часть 3: Читаем еще 127 секторов (для запаса)
    add bx, 0xFE00      ; Сдвигаем адрес еще раз
    mov dh, 127
    mov dl, [BOOT_DRIVE]
    mov cl, 256         ; Начинаем с 256-го сектора (129 + 127)
    call disk_load

    ; --- 2. НАСТРОЙКА VESA ---
    mov ax, 0x4F01
    mov cx, 0x4143      ; 800x600x32
    mov di, 0x7000
    int 0x10

    mov eax, [0x7028]
    mov [vbe_fb], eax

    mov ax, 0x4F02
    mov bx, 0x4143
    int 0x10

    ; --- 3. ПЕРЕХОД В 32-БИТНЫЙ РЕЖИМ ---
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
    
    push dword [vbe_fb]
    jmp KERNEL_OFFSET
    jmp $

disk_load:
    push dx
    mov ah, 0x02
    mov al, dh
    mov ch, 0
    mov dh, 0
    int 0x13
    jc disk_error
    pop dx
    ret

disk_error:
    jmp $

; --- GDT и прочее ---
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
vbe_fb     dd 0

times 510-($-$$) db 0
dw 0xaa55