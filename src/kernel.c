#include "io/io.h"
#include "drivers/screen/screen.h"
#include "drivers/keyboard/keyboard.h"
#include "system/gdt.h"
#include "system/idt.h"
#include "system/pic.h"

// Глобальные переменные (доступны для screen.c через extern)
int current_col = 0;
int current_row = 0;

char key_buffer[256];
int buffer_idx = 0;

void keyboard_callback() {
    unsigned char scancode = inb(0x60);
    char c = get_ascii_char(scancode);
    
    if (c != 0) {
        if (c == '\b') {
            if (buffer_idx > 0) {
                buffer_idx--;
                key_buffer[buffer_idx] = '\0';
                
                // Шаг назад, стираем, и снова назад (так как kprint подвинет курсор вперед)
                current_col--;
                if (current_col < 0) current_col = 0;
                kprint_at(" ", current_col, current_row);
                current_col--; 
                update_cursor(current_col, current_row);
            }
        } else if (c == '\n') {
            key_buffer[buffer_idx] = '\0';
            buffer_idx = 0;
            kprint("\n> "); // kprint сама сделает row++ и col=2
        } else {
            if (buffer_idx < 255) {
                key_buffer[buffer_idx++] = c;
                char str[2] = {c, 0};
                kprint(str); // Просто печатаем, всё обновится само
            }
        }
    }
}

void kmain() {
    init_gdt();
    pic_remap();
    init_idt();
    
    clear_screen(); // Обнулит координаты в 0,0
    
    kprintf("EquinoxOS Alpha is running!\n");
    kprintf("Mode: %s | Kernel Base: 0x%d\n", "32-bit", 0x1000);
    kprint("Type something...\n> ");
    
    // Координаты уже установлены функцией kprint автоматически

    while(1) {
        __asm__ __volatile__("hlt");
    }
}