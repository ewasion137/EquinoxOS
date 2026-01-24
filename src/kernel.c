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
    
    clear_screen();

    // --- ТЕСТ ДИСКА ---
    kprint("Checking Storage Drive... ");
    unsigned char test_sector[512];
    // Читаем 0-й сектор Slave диска (fs.img)
    read_sectors_ata_pio((uint32_t)test_sector, 0, 1);
    kprint("OK!\n");
    // ------------------

    kprint("> ");
    
    while(1) { __asm__ __volatile__("hlt"); }
}