// vga.c
#include "vga.h"
#include "../../io/io.h"
#include "font8x8.h"
extern const uint8_t _binary_src_vga_font_psf_start[];

#define VGA_MISC_WRITE 0x3C2
#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA  0x3D5
#define VGA_SEQ_INDEX  0x3C4
#define VGA_SEQ_DATA   0x3C5
#define VGA_GC_INDEX   0x3CE
#define VGA_GC_DATA    0x3CF
#define VGA_AC_INDEX   0x3C0
#define VGA_AC_WRITE   0x3C0
#define VGA_AC_READ    0x3C1

void write_regs(unsigned char *regs) {
    // Эта функция нам больше не нужна в старом виде,
    // мы напишем явную инициализацию ниже.
}

void init_vga() {
    // 1. MISC Register
    outb(VGA_MISC_WRITE, 0x63);

    // 2. Sequencer Registers
    outb(VGA_SEQ_INDEX, 0); outb(VGA_SEQ_DATA, 0x03); // Reset
    outb(VGA_SEQ_INDEX, 1); outb(VGA_SEQ_DATA, 0x01); // Clocking Mode
    outb(VGA_SEQ_INDEX, 2); outb(VGA_SEQ_DATA, 0x0F); // Map Mask
    outb(VGA_SEQ_INDEX, 3); outb(VGA_SEQ_DATA, 0x00); // Char Map Select
    outb(VGA_SEQ_INDEX, 4); outb(VGA_SEQ_DATA, 0x0E); // Memory Mode

    outb(VGA_SEQ_INDEX, 0); outb(VGA_SEQ_DATA, 0x03); // Reset end

    // 3. CRTC Registers (Снятие защиты и настройка таймингов)
    outb(VGA_CRTC_INDEX, 0x03); outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) | 0x80); // Unlock
    outb(VGA_CRTC_INDEX, 0x11); outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) & ~0x80); // Unlock

    unsigned char crtc_vals[] = {
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
        0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
        0xFF
    };

    for(int i = 0; i <= 0x18; i++) {
        outb(VGA_CRTC_INDEX, i);
        outb(VGA_CRTC_DATA, crtc_vals[i]);
    }

    // 4. Graphics Controller Registers
    outb(VGA_GC_INDEX, 0); outb(VGA_GC_DATA, 0x00); // Set/Reset
    outb(VGA_GC_INDEX, 1); outb(VGA_GC_DATA, 0x00); // Enable Set/Reset
    outb(VGA_GC_INDEX, 2); outb(VGA_GC_DATA, 0x00); // Color Compare
    outb(VGA_GC_INDEX, 3); outb(VGA_GC_DATA, 0x00); // Data Rotate
    outb(VGA_GC_INDEX, 4); outb(VGA_GC_DATA, 0x00); // Read Map Select
    outb(VGA_GC_INDEX, 5); outb(VGA_GC_DATA, 0x40); // Mode Register (256 color)
    outb(VGA_GC_INDEX, 6); outb(VGA_GC_DATA, 0x05); // Misc
    outb(VGA_GC_INDEX, 7); outb(VGA_GC_DATA, 0x0F); // Color Don't Care
    outb(VGA_GC_INDEX, 8); outb(VGA_GC_DATA, 0xFF); // Bit Mask

    // 5. Attribute Controller Registers
    inb(0x3DA); // Reset flip-flop
    unsigned char ac_vals[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x41, 0x00, 0x0F, 0x00, 0x00
    };

    for(int i = 0; i <= 0x14; i++) {
        inb(0x3DA); // Reset flip-flop before every write (safety first)
        outb(VGA_AC_INDEX, i);
        outb(VGA_AC_WRITE, ac_vals[i]);
    }
    
    inb(0x3DA);
    outb(VGA_AC_INDEX, 0x20); // Enable video
}

// Рисуем пиксель
void put_pixel(int x, int y, uint8_t color) {
    // В режиме 320x200 память видеокарты линейна
    uint8_t* screen = (uint8_t*)0xA0000;
    if (x >= 0 && x < 320 && y >= 0 && y < 200) {
        screen[y * 320 + x] = color;
    }
}

// Заливка экрана
void clear_screen_vga(uint8_t color) {
    uint8_t* screen = (uint8_t*)0xA0000;
    for (int i = 0; i < 320 * 200; i++) {
        screen[i] = color;
    }
}

// Рисование квадрата
void draw_rect(int x, int y, int w, int h, uint8_t color) {
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) {
            put_pixel(j, i, color);
        }
    }
}

void draw_char(char c, int x, int y, uint8_t fg, uint8_t bg) {
    // Если символ вне диапазона ASCII, ничего не рисуем
    if (c < 0 || c > 127) return;

    // Цикл по 8 строкам символа
    for (int i = 0; i < 8; i++) {
        // Берем байт строки из нашего встроенного массива
        uint8_t row = font8x8_basic[(int)c][i];
        
        // Цикл по 8 битам (пикселям) в строке
        for (int j = 0; j < 8; j++) {
            // Проверяем бит. В font8x8 бит 0 - это левый пиксель.
            if (row & (1 << j)) {
                put_pixel(x + j, y + i, fg);
            } else if (bg != 255) { // 255 - "прозрачный"
                put_pixel(x + j, y + i, bg);
            }
        }
    }
}

void draw_string(const char* s, int x, int y, uint8_t fg, uint8_t bg) {
    int x_offset = 0;
    while (*s) {
        // Рисуем текущий символ
        draw_char(*s, x + x_offset, y, fg, bg);
        // Переходим к следующему символу (8 пикселей вправо)
        x_offset += 8;
        s++;
    }
}