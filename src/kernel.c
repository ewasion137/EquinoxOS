#include "io/io.h"
#include "drivers/screen/screen.h"
#include "drivers/keyboard/keyboard.h"
#include "system/gdt.h"
#include "system/idt.h"
#include "system/pic.c"

// Переменные для отслеживания позиции печати
int current_col = 0;
int current_row = 5; // Начнем с 5-й строки, чтобы не затереть шапку

void kmain() {
    init_gdt();
    pic_remap();
    init_idt();
    clear_screen();
    kprint_at("GDT Initialized. Memory protection layer ready.", 0, 1);
    kprint_at("EquinoxOS v0.0.1 - Type something!", 0, 0);
    kprint_at("----------------------------------", 0, 1);
    kprint_at("IDT Ready", 0, 1);

    while(1) {
    if (inb(0x64) & 0x01) { // Если в буфере клавиатуры что-то есть
        unsigned char scancode = inb(0x60);
        char c = get_ascii_char(scancode); // Драйвер сам разберется с Шифтом
        
        // Печатаем, только если это реально символ (не 0)
        if (c != 0) {
            if (c == '\b') {
                // Твой код для бэкспейса
                if (current_col > 0) {
                        current_col--;
                        kprint_at(" ", current_col, current_row);
                        update_cursor(current_col, current_row);
                }
                } else if (c == '\n') {
                    // Твой код для энтера
                    current_col = 0;
                    current_row++;
                    update_cursor(current_col, current_row);
                } else {
                    // Обычная печать
                    char str[2] = {c, 0};
                    kprint_at(str, current_col, current_row);
                    current_col++;
                    if (current_col >= 80) { current_col = 0; current_row++; }
                    update_cursor(current_col, current_row);
                }
            }
        }
    }
}