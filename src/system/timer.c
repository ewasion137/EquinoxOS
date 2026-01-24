#include "timer.h"
#include "io/io.h"
#include "../drivers/screen/screen.h"

uint32_t tick = 0;
extern uint32_t tick;

void timer_callback() {
    tick++;

    // Каждые 100 тиков (раз в секунду) обновляем счетчик в углу
    if (tick % 100 == 0) {
        uint32_t sec = tick / 100;
        
        // Сохраняем текущие координаты, чтобы не сбить ввод пользователя
        extern int current_col;
        extern int current_row;
        int old_x = current_col;
        int old_y = current_row;

        // Рисуем время в самом углу (строка 0, колонка 70)
        kprint_at("TIME: ", 65, 0);
        char buf[16];
        itoa(sec, buf);
        kprint_at(buf, 71, 0);

        // Возвращаем курсор на место, где пользователь пишет
        update_cursor(old_x, old_y);
        current_col = old_x;
        current_row = old_y;
    }
}

void init_timer(uint32_t freq) {
    uint32_t divisor = 1193180 / freq;
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

void sleep(uint32_t ms) {
    // Если таймер работает на 100Гц, то 1 тик = 10мс
    // Поэтому ждем (ms / 10) тиков
    uint32_t end = tick + (ms / 10);
    while (tick < end) {
        // Просто ждем. Процессор будет переходить в прерывание, 
        // увеличивать tick и возвращаться сюда.
        __asm__ __volatile__("hlt"); 
    }
}