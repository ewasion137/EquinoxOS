#ifndef VESA_H
#define VESA_H

#include <stdint.h>

// Основные параметры экрана (совпадают с тем, что мы задали в boot.asm)
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Цвета в формате 0xRRGGBB (TrueColor)
#define COLOR_BLACK       0x000000
#define COLOR_WHITE       0xFFFFFF
#define COLOR_RED         0xFF0000
#define COLOR_GREEN       0x00FF00
#define COLOR_BLUE        0x0000FF
#define COLOR_AERO_BLUE   0x0078D7  // Тот самый синий из Windows
#define COLOR_GREY        0xCCCCCC
#define COLOR_DARK_GREY   0x333333

// Прототипы функций
void init_vesa(uint32_t fb_addr);
void put_pixel(int x, int y, uint32_t color);
void draw_rect(int x, int y, int w, int h, uint32_t color);
void draw_background(); // Функция для градиента
void vesa_draw_string(const char* s, int x, int y, uint32_t fg);
void vesa_draw_char(char c, int x, int y, uint32_t fg);
void draw_transparent_rect(int x, int y, int w, int h, uint32_t color, uint8_t alpha);

#endif