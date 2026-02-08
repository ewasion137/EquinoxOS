// timer.c
#include "timer.h"
#include "io/io.h"

// volatile крайне важен, чтобы компилятор не оптимизировал проверки в sleep()
volatile uint32_t tick = 0; 

void timer_callback() {
    tick++;
    // Мы убрали отсюда отрисовку. 
    // В прерываниях (ISR) лучше не делать сложную логику вывода на экран,
    // так как это может замедлить систему или вызвать зависание.
}

void init_timer(uint32_t freq) {
    uint32_t divisor = 1193180 / freq;
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

void sleep(uint32_t ms) {
    // Если таймер 100Гц, то 1 тик = 10мс.
    uint32_t start_tick = tick;
    uint32_t wait_ticks = ms / 10;
    while (tick < start_tick + wait_ticks) {
        __asm__ __volatile__("hlt"); 
    }
}