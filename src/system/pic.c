// src/system/pic.c - УБЕДИСЬ, ЧТО ЭТОТ КОД АКТУАЛЕН!
#include "io/io.h" 

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define ICW1_ICW4	0x01
#define ICW1_SINGLE	0x02
#define ICW1_INTERVAL4	0x04
#define ICW1_LEVEL	0x08
#define ICW1_INIT	0x10
 
#define ICW4_8086	0x01
#define ICW4_AUTO	0x02
#define ICW4_BUF_SLAVE	0x08
#define ICW4_BUF_MASTER	0x0C
#define ICW4_SFNM	0x10

void pic_remap() {
    uint8_t a1, a2;

    a1 = inb(PIC1_DATA); // Сохраняем маски
    a2 = inb(PIC2_DATA);

    // ICW1 - Начало инициализации
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); 
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    // ICW2 - Переназначение векторов
    outb(PIC1_DATA, 0x20); // Master IRQs 0-7 -> IDT 32-39
    outb(PIC2_DATA, 0x28); // Slave IRQs 8-15 -> IDT 40-47

    // ICW3 - Каскадирование
    outb(PIC1_DATA, 0x04); // Master: Slave на IRQ2
    outb(PIC2_DATA, 0x02); // Slave: Его ID 2

    // ICW4 - Режим работы
    outb(PIC1_DATA, ICW4_8086); // 8086 режим
    outb(PIC2_DATA, ICW4_8086); // 8086 режим

    outb(PIC1_DATA, 0xFF); // Замаскировать все Master IRQ
    outb(PIC2_DATA, 0xFF); // Замаскировать все Slave IRQ
    
    // РАЗМАСКИРОВЫВАЕМ НУЖНЫЕ (Master PIC)
    // IRQ0 (таймер), IRQ1 (клавиатура), IRQ2 (для каскада Slave)
    // 0xFF - (1<<0) - (1<<1) - (1<<2)
    outb(PIC1_DATA, ~( (1<<0) | (1<<1) | (1<<2) ) & a1); // Разрешить IRQ0, IRQ1, IRQ2

    // РАЗМАСКИРОВЫВАЕМ НУЖНЫЕ (Slave PIC)
    // IRQ12 (мышь)
    // 0xFF - (1<<4)
    outb(PIC2_DATA, ~(1<<4) & a2); // Разрешить IRQ12
}