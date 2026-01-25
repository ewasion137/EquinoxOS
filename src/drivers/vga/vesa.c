#include <stdint.h>
#include "font8x8.h"

uint32_t* framebuffer;
int screen_width = 800;
int screen_height = 600;

void init_vesa(uint32_t fb_addr) {
    framebuffer = (uint32_t*)fb_addr;
}

// Теперь цвет - это 0xRRGGBB
void put_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < screen_width && y >= 0 && y < screen_height) {
        framebuffer[y * screen_width + x] = color;
    }
}

void vesa_clear_screen(uint32_t color) {
    for (int i = 0; i < screen_width * screen_height; i++) {
        framebuffer[i] = color;
    }
}

// Рисуем градиент (первый шаг к Frutiger Aero!)
void draw_background() {
    for (int y = 0; y < screen_height; y++) {
        for (int x = 0; x < screen_width; x++) {
            // Плавный переход от темно-синего к голубому
            uint32_t blue = 100 + (y / 6); 
            put_pixel(x, y, (blue << 0) | (50 << 8) | (30 << 16));
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
            // bg не рисуем — так текст будет "прозрачным"
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

// alpha: 0 - полностью прозрачный, 255 - глухой цвет
uint32_t blend(uint32_t color_bg, uint32_t color_fg, uint8_t alpha) {
    uint32_t rb = (((color_fg & 0xFF00FF) * alpha) + ((color_bg & 0xFF00FF) * (255 - alpha))) >> 8;
    uint32_t g  = (((color_fg & 0x00FF00) * alpha) + ((color_bg & 0x00FF00) * (255 - alpha))) >> 8;
    return (rb & 0xFF00FF) | (g & 0x00FF00);
}

// Рисование полупрозрачного прямоугольника
void draw_transparent_rect(int x, int y, int w, int h, uint32_t color, uint8_t alpha) {
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) {
            uint32_t bg_color = framebuffer[i * screen_width + j];
            put_pixel(j, i, blend(bg_color, color, alpha));
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