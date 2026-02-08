// src/drivers/mouse/mouse.h
#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

// Глобальные переменные для хранения состояния мыши
// ОБЯЗАТЕЛЬНО volatile, так как они меняются в прерывании и читаются в основном цикле
extern volatile int32_t mouse_x;
extern volatile int32_t mouse_y;
extern volatile uint8_t mouse_left_button;
extern volatile uint8_t mouse_right_button;

// Функция инициализации PS/2 мыши
void init_mouse();

#endif