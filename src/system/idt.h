#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// Структура записи в IDT
typedef struct {
    uint16_t low_offset; // Нижние 16 бит адреса обработчика
    uint16_t sel;        // Сегмент кода (0x08)
    uint8_t  always0;
    uint8_t  flags;      // Флаги (0x8E - прерывание присутствует, 32-бит)
    uint16_t high_offset; // Верхние 16 бит адреса
} __attribute__((packed)) idt_gate_t;

// Указатель на IDT
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_register_t;

void set_idt_gate(int n, uint32_t handler);
extern void irq12();
void init_idt();

#endif