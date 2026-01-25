#ifndef VGA_H
#define VGA_H

#include <stdint.h>

#define VGA_ADDRESS 0xA0000
#define VGA_WIDTH 320
#define VGA_HEIGHT 200

// Цвета (в палитре 256 цветов)
#define COLOR_BLACK 0
#define COLOR_BLUE 1
#define COLOR_GREEN 2
#define COLOR_CYAN 3
#define COLOR_RED 4
#define COLOR_MAGENTA 5
#define COLOR_BROWN 6
#define COLOR_WHITE 15
// ... остальные цвета от 16 до 255 - это оттенки

void init_vga();
void put_pixel(int x, int y, uint8_t color);
void draw_rect(int x, int y, int w, int h, uint8_t color);
void clear_screen_vga(uint8_t color);
void draw_char(char c, int x, int y, uint8_t fg, uint8_t bg);
void draw_string(const char* s, int x, int y, uint8_t fg, uint8_t bg);

#endif