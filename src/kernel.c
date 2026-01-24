#include "io/io.h"
#include "drivers/screen/screen.h"
#include "drivers/keyboard/keyboard.h"
#include "system/gdt.h"
#include "system/idt.h"
#include "system/pic.h"

// Глобальные переменные для курсора
int current_col = 0;
int current_row = 5;

// ЭТА ФУНКЦИЯ ВЫЗЫВАЕТСЯ ИЗ АССЕМБЛЕРА ПРИ НАЖАТИИ
void keyboard_callback() {
    unsigned char scancode = inb(0x60);
    char c = get_ascii_char(scancode);
    
    if (c != 0) {
        if (c == '\b') {
            if (current_col > 0) {
                current_col--;
                kprint_at(" ", current_col, current_row);
                update_cursor(current_col, current_row);
            }
        } else if (c == '\n') {
            current_col = 0;
            current_row++;
            update_cursor(current_col, current_row);
        } else {
            char str[2] = {c, 0};
            kprint_at(str, current_col, current_row);
            current_col++;
            if (current_col >= 80) { current_col = 0; current_row++; }
            update_cursor(current_col, current_row);
        }
    }
}

void kmain() {
    init_gdt();
    pic_remap();
    init_idt();
    
    clear_screen();
    kprint_at("EquinoxOS Alpha - Interrupt Driven Mode", 0, 0);
    kprint_at("---------------------------------------", 0, 1);
    kprint_at("Now typing works via IDT (IRQ 1).", 0, 3);
    update_cursor(current_col, current_row);

    while(1); // Просто спим. Весь движ теперь в keyboard_callback!
}