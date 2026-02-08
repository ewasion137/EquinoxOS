#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// Инициализация таймера (PIT)
void init_timer(uint32_t freq);

// Ожидание в мс
void sleep(uint32_t ms);

// volatile говорит компилятору: "это значение меняется извне (в прерывании), 
// не пытайся оптимизировать проверки этой переменной!"
extern volatile uint32_t tick; 

#endif