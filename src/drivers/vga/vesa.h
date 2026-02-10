#ifndef VESA_H
#define VESA_H

#include <stdint.h>

// Теперь эти значения будут устанавливаться динамически из Limine
extern uintptr_t fb_base_addr;
extern uint32_t screen_width;
extern uint32_t screen_height;
extern uint32_t screen_pitch;

// Цвета в формате 0xRRGGBB (32 бита достаточно)
#define COLOR_BLACK       0x000000
#define COLOR_WHITE       0xFFFFFF
#define COLOR_RED         0xFF0000
#define COLOR_GREEN       0x00FF00
#define COLOR_BLUE        0x0000FF
#define COLOR_AERO_BLUE   0x0078D7
#define COLOR_GREY        0xCCCCCC
#define COLOR_DARK_GREY   0x333333

// Прототипы функций с правильными аргументами
void init_vesa(uint64_t fb_addr, uint32_t width, uint32_t height, uint32_t pitch);
void put_pixel(int x, int y, uint32_t color);
void draw_rect(int x, int y, int w, int h, uint32_t color);
void draw_background();
void vesa_draw_string(const char* s, int x, int y, uint32_t fg);
void vesa_draw_char(char c, int x, int y, uint32_t fg);
void draw_transparent_rect(int x, int y, int w, int h, uint32_t color, uint8_t alpha);
void vesa_draw_string_hex(const char* prefix, int x, int y, uint64_t val, uint32_t fg);
void hex_to_string(uint64_t val, char* buf);

#endif