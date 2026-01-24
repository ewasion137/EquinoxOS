#include "idt.h"

idt_gate_t idt[256];
idt_register_t idt_reg;

extern void keyboard_handler(); // Наш новый спец-обработчик из асма
extern void isr_stub();         // Общая заглушка для остальных

void set_idt_gate(int n, uint32_t handler) {
    idt[n].low_offset = (uint16_t)(handler & 0xFFFF);
    idt[n].sel = 0x08;
    idt[n].always0 = 0;
    idt[n].flags = 0x8E;
    idt[n].high_offset = (uint16_t)((handler >> 16) & 0xFFFF);
}

void init_idt() {
    idt_reg.base = (uint32_t)&idt;
    idt_reg.limit = 256 * sizeof(idt_gate_t) - 1;

    for (int i = 0; i < 256; i++) set_idt_gate(i, (uint32_t)isr_stub);

    // РЕГИСТРИРУЕМ КЛАВИАТУРУ (IRQ1 = 32 + 1 = 33)
    set_idt_gate(33, (uint32_t)keyboard_handler);

    __asm__ __volatile__("lidtl (%0)" : : "r" (&idt_reg));
    __asm__ __volatile__("sti");
}