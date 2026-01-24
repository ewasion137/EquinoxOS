#include "io/io.h"
#include "drivers/screen/screen.h"
#include "drivers/keyboard/keyboard.h"
#include "system/gdt.h"
#include "system/idt.h"
#include "system/pic.h"

void kmain() {
    init_gdt();
    pic_remap(); // Сначала перенаправляем сигналы железа
    init_idt();  // Потом включаем их обработку
    
    clear_screen();
    kprint_at("EquinoxOS: Interrupts are ACTIVE!", 0, 1);
    kprint_at("Try typing - polling is still working, but interrupts are firing behind the scenes.", 0, 1);

    while(1); // Теперь это не просто пустой цикл, он ждет прерываний
}