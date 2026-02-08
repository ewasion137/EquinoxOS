#include "idt.h"

idt_gate_t idt[256];
idt_register_t idt_reg;

extern void keyboard_handler(); // Наш новый спец-обработчик из асма
extern void isr_stub();         // Общая заглушка для остальных
extern void timer_handler();
extern void mouse_handler();

void set_idt_gate(int n, uint64_t handler) {
    idt[n].low_offset = (uint16_t)(handler & 0xFFFF);
    idt[n].sel = 0x08;
    idt[n].always0 = 0;
    idt[n].flags = 0x8E;
    idt[n].high_offset = (uint16_t)((handler >> 16) & 0xFFFF);
    idt[n].very_high_offset = (uint32_t)(handler >> 32); 
    idt[n].reserved = 0;
}

void init_idt() {
    idt_reg.base = (uint64_t)&idt;
    idt_reg.limit = 256 * sizeof(idt_gate_t) - 1;

    for (int i = 0; i < 256; i++) set_idt_gate(i, (uint64_t)isr_stub);

    // РЕГИСТРИРУЕМ ТАЙМЕР (IRQ0 = 32)
    set_idt_gate(32, (uint64_t)timer_handler);
    // РЕГИСТРИРУЕМ КЛАВИАТУРУ (IRQ1 = 33)
    set_idt_gate(33, (uint64_t)keyboard_handler);
    // РЕГИСТРИРУЕМ МЫШЬ (IRQ12 = 32 + 12 = 44)
    set_idt_gate(44, (uint64_t)mouse_handler); // <-- ДОБАВЬ ЭТУ СТРОКУ

    __asm__ __volatile__("lidt (%0)" : : "r" (&idt_reg));
    __asm__ __volatile__("sti"); // Включаем прерывания
}