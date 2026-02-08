// src/system/pic.c
#include "io/io.h" // Убедись, что это включено

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define ICW1_ICW4	0x01		/* ICW4 (not) present */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */
 
#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

void pic_remap() {
    uint8_t a1, a2;

    // Сохраняем текущие маски
    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);

    // ICW1 - Начинаем инициализацию (Master PIC)
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); 
    // Задержка (короткая)
    // outb(0x80, 0); // Обычно используется для задержки, но QEMU может не требовать

    // ICW1 - Начинаем инициализацию (Slave PIC)
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    // Задержка
    // outb(0x80, 0);

    // ICW2 - Переназначение векторов
    outb(PIC1_DATA, 0x20); // Master PIC IRQs (0-7) -> IDT entries 0x20-0x27 (32-39)
    // Задержка
    // outb(0x80, 0);
    outb(PIC2_DATA, 0x28); // Slave PIC IRQs (8-15) -> IDT entries 0x28-0x2F (40-47)
    // Задержка
    // outb(0x80, 0);

    // ICW3 - Каскадирование
    outb(PIC1_DATA, 0x04); // Master PIC: Slave на IRQ2 (0000 0100)
    // Задержка
    // outb(0x80, 0);
    outb(PIC2_DATA, 0x02); // Slave PIC: Его идентификатор 2
    // Задержка
    // outb(0x80, 0);

    // ICW4 - Режим работы
    outb(PIC1_DATA, ICW4_8086); // 8086 режим для Master
    // Задержка
    // outb(0x80, 0);
    outb(PIC2_DATA, ICW4_8086); // 8086 режим для Slave
    // Задержка
    // outb(0x80, 0);

    // МАСКИРУЕМ ВСЕ ПРЕРЫВАНИЯ СРАЗУ ПОСЛЕ РЕМПАППИНГА
    // Это предотвратит Triple Fault от непроинициализированных обработчиков
    outb(PIC1_DATA, 0xFF); // Замаскировать все IRQ на Master PIC
    outb(PIC2_DATA, 0xFF); // Замаскировать все IRQ на Slave PIC
    
    // Восстанавливаем старые маски (или устанавливаем нужные)
    // outb(PIC1_DATA, a1); // Можно восстановить старые, но лучше задать свои
    // outb(PIC2_DATA, a2);

    // Для начала: размаскируем только те, что нам нужны:
    // IRQ0 (таймер) = 0x20 + 0 = 32
    // IRQ1 (клавиатура) = 0x20 + 1 = 33
    // IRQ12 (мышь) = 0x28 + 4 = 44 (или 0x20 + 12 = 44)
    // Маска: (1 << IRQ)
    // Не забываем про IRQ2 (0x04 на Master), который каскадирует Slave PIC
    outb(PIC1_DATA, ~(1<<0 | 1<<1 | 1<<2)); // Разрешить IRQ0, IRQ1, IRQ2
    outb(PIC2_DATA, ~(1<<4));               // Разрешить IRQ12 (т.е. бит 4 в Slave PIC)
}