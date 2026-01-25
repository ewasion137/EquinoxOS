#ifndef MOUSE_H
#define MOUSE_H
#include <stdint.h>

void mouse_wait(uint8_t type);
void mouse_write(uint8_t a);
void init_mouse();
void mouse_handler();
#endif