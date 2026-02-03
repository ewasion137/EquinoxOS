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
#include "boot/limine/limine.h"
#include <stdint.h>
#include <stddef.h>

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID, // Тот самый ID из хедера
    .revision = 0
};

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

void kmain(void) {
    // Проверяем, дал ли нам загрузчик ответ по экрану
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        // Если экрана нет — зависаем (на реальном железе можно мигнуть диодом)
        while(1);
    }

    // Получаем адрес экрана из ответа Limine
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    uint64_t fb_addr = (uint64_t)fb->address;

    init_gdt();
    init_vesa(fb_addr);

    // Твой старый код отрисовки
    draw_background(); 
    draw_transparent_rect(100, 100, 600, 400, 0xFFFFFF, 150);
    draw_rect(100, 100, 600, 30, 0x0055AA);
    vesa_draw_string("EquinoxOS - Limine Power", 110, 110, 0xFFFFFF);
    vesa_draw_string("Booted via Limine. BIOS legacy discarded.", 120, 150, 0x000000);

    while(1) { __asm__ __volatile__("hlt"); }
}