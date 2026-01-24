#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// Инициализация таймера с частотой (например, 100 Гц)
void init_timer(uint32_t freq);

// Функция ожидания в миллисекундах
void sleep(uint32_t ms);

// Счетчик тиков (может пригодиться для дебага)
extern uint32_t tick;

#endif