#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// Структура записи в IDT
typedef struct {
    uint16_t low_offset;   // 0-15 бит адреса обработчика
    uint16_t sel;          // Сегментный селектор
    uint8_t  always0;      // Всегда 0
    uint8_t  flags;        // Флаги (P, DPL, S, Type)
    uint16_t high_offset;  // 16-31 бит адреса обработчика
    uint32_t very_high_offset; // НОВОЕ! 32-63 бит адреса обработчика
    uint32_t reserved;     // Зарезервировано (Всегда 0)
} __attribute__((packed)) idt_gate_t;

// Указатель на IDT
typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idt_register_t;

void set_idt_gate(int n, uint64_t handler);
extern void irq12();
void init_idt();

#endif