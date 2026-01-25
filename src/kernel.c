// kernel.c
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
#include "drivers/vga/vesa.h"

// Глобальные координаты (объявлены здесь, используются везде через extern)
int current_col = 0;
int current_row = 0;
extern int mouse_x;
extern int mouse_y;
extern void init_mouse();

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

void kmain(uint32_t fb_addr) {
    init_gdt();
    init_idt();
    pic_remap();
    init_vesa(fb_addr);
    
    // 1. Сначала один раз готовим фон
    init_background(); 
    
    init_mouse();
    __asm__("sti");

    while(1) {
        // 2. Копируем чистый фон в буфер (стирает всё старое)
        vesa_prepare_frame();

        // 3. Рисуем интерфейс Aero
        draw_transparent_rect(150, 100, 500, 350, 0xFFFFFF, 160);
        draw_rect(150, 100, 500, 30, 0x0055AA);
        vesa_draw_string("EquinoxOS - Aero Glass Environment", 160, 110, 0xFFFFFF);

        // 4. Рисуем мышь ПОВЕРХ окна
        draw_cursor(mouse_x, mouse_y);

        // 5. Выкидываем всё на монитор
        vesa_update(); 
    }
}