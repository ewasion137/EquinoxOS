// src/system/idt.h
#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// Структура записи в IDT для 64-битного режима
typedef struct {
    uint16_t low_offset;    // 0-15 бит адреса обработчика
    uint16_t sel;           // Сегментный селектор (Limine обычно 0x08 или 0x28)
    uint8_t  ist;           // Interrupt Stack Table (устанавливается в 0, если не используется)
    uint8_t  flags;         // Флаги (P, DPL, S, Type), обычно 0x8E для прерываний
    uint16_t mid_offset;    // 16-31 бит адреса обработчика
    uint32_t high_offset;   // 32-63 бит адреса обработчика
    uint32_t reserved;      // Зарезервировано (Всегда 0)
} __attribute__((packed)) idt_gate_t; // __attribute__((packed)) критически важен!

// Указатель на IDT
typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idt_register_t;

void set_idt_gate(int n, uint64_t handler);
// extern void irq12(); // Эту строку можно удалить, если не используется напрямую

#endif