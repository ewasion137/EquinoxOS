// vesa.c
#include "vesa.h"
#include "font8x8.h"

// Используем uintptr_t для хранения адреса, чтобы не было путаницы с размером типа
uintptr_t fb_base_addr; 
uint32_t screen_width;
uint32_t screen_height;
uint32_t screen_pitch;

void init_vesa(uint64_t fb_addr, uint32_t width, uint32_t height, uint32_t pitch) {
    fb_base_addr = (uintptr_t)fb_addr;
    screen_width = width;
    screen_height = height;
    screen_pitch = pitch;
}

void put_pixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= (int)screen_width || y < 0 || y >= (int)screen_height) return;

    // Считаем точный адрес байта:
    // база + (Y * байт_в_строке) + (X * 4 байта на пиксель)
    uint32_t* pixel_ptr = (uint32_t*)(fb_base_addr + (y * screen_pitch) + (x * 4));
    *pixel_ptr = color;
}

void draw_background() {
    for (int y = 0; y < (int)screen_height; y++) {
        for (int x = 0; x < (int)screen_width; x++) {
            uint32_t blue = 100 + (y / 10); 
            put_pixel(x, y, (blue << 0) | (50 << 8) | (30 << 16));
        }
    }
}

void draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) {
            put_pixel(j, i, color);
        }
    }
}

// Вспомогательная функция для смешивания цветов
uint32_t blend(uint32_t color_bg, uint32_t color_fg, uint8_t alpha) {
    uint32_t rb = (((color_fg & 0xFF00FF) * alpha) + ((color_bg & 0xFF00FF) * (255 - alpha))) >> 8;
    uint32_t g  = (((color_fg & 0x00FF00) * alpha) + ((color_bg & 0x00FF00) * (255 - alpha))) >> 8;
    return (rb & 0xFF00FF) | (g & 0x00FF00);
}

void draw_transparent_rect(int x, int y, int w, int h, uint32_t color, uint8_t alpha) {
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) {
            if (j >= 0 && j < (int)screen_width && i >= 0 && i < (int)screen_height) {
                // Читаем фоновый цвет напрямую по адресу
                uint32_t* pixel_ptr = (uint32_t*)(fb_base_addr + (i * screen_pitch) + (j * 4));
                uint32_t bg_color = *pixel_ptr;
                put_pixel(j, i, blend(bg_color, color, alpha));
            }
        }
    }
}

void vesa_draw_char(char c, int x, int y, uint32_t fg) {
    if (c < 0 || c > 127) return;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (font8x8_basic[(int)c][i] & (1 << j)) {
                put_pixel(x + j, y + i, fg);
            }
        }
    }
}

void vesa_draw_string(const char* s, int x, int y, uint32_t fg) {
    while (*s) {
        vesa_draw_char(*s, x, y, fg);
        x += 8;
        s++;
    }
}