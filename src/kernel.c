#include "io/io.h"
#include "drivers/screen/screen.h"
#include "drivers/keyboard/keyboard.h"
#include "system/gdt.h"
#include "system/idt.h"
#include "system/pic.h"
#include "shell/shell.h"
#include "../system/timer.h"
#include "drivers/disk/ata.h"
#include "../fs/fs.h"
#include "drivers/vga/vga.h"

// Глобальные координаты (объявлены здесь, используются везде через extern)
int current_col = 0;
int current_row = 0;

// Буфер для ввода команд
char key_buffer[256];
int buffer_idx = 0;

void keyboard_callback() {
    unsigned char scancode = inb(0x60);
    char c = get_ascii_char(scancode);
    
    if (c != 0) {
        // Стирание символа
        if (c == '\b') {
            if (buffer_idx > 0) {
                buffer_idx--;
                key_buffer[buffer_idx] = '\0';
                
                current_col--;
                if (current_col < 0) current_col = 0;
                
                kprint_at(" ", current_col, current_row);
                current_col--; // Корректируем после kprint_at
                update_cursor(current_col, current_row);
            }
        } 
        // Выполнение команды
        else if (c == '\n') {
            key_buffer[buffer_idx] = '\0';
            kprint("\n");
            
            execute_command(key_buffer);
            
            buffer_idx = 0;
            kprint("> ");
        } 
        // Набор текста
        else {
            if (buffer_idx < 255) {
                key_buffer[buffer_idx++] = c;
                char str[2] = {c, 0};
                kprint(str);
            }
        }
    }
}

void kmain() {
    init_gdt();
    pic_remap();
    init_idt();
    init_timer(100);
    
    // Включаем графику
    init_vga();
    
    // 1. Заливаем фон темно-синим (или черным)
    clear_screen_vga(1); // 1 = Blue
    
    // ТЕСТ: Рисуем цветные полосы для проверки работы VGA
    for (int i = 0; i < 320; i++) {
        for (int j = 0; j < 20; j++) {
            put_pixel(i, j, 15); // Белая полоса сверху
        }
    }
    for (int i = 0; i < 320; i++) {
        for (int j = 20; j < 40; j++) {
            put_pixel(i, j, 4); // Красная полоса
        }
    }
    
    // 2. Рисуем "Окно" по центру (стиль Win7 Basic)
    // x=60, y=50, ширина=200, высота=100
    draw_rect(60, 50, 200, 100, 7);
    draw_rect(60, 50, 200, 15, 9);
    
    // ТЕКСТ (Цвет 15 - Белый)
    draw_string(70, 54, "EquinoxOS Setup", 15);
    draw_string(70, 80, "Installing drivers...", 15); // Изменено на белый для видимости

    while(1) { __asm__ __volatile__("hlt"); }
}