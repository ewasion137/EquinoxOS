#include "screen.h"
#include "../../io/io.h"

// Функция обновления позиции мигающего курсора
void update_cursor(int x, int y) {
    unsigned short pos = y * MAX_COLS + x;
    outb(REG_SCREEN_CTRL, 0x0F);
    outb(REG_SCREEN_DATA, (unsigned char) (pos & 0xFF));
    outb(REG_SCREEN_CTRL, 0x0E);
    outb(REG_SCREEN_DATA, (unsigned char) ((pos >> 8) & 0xFF));
}

void clear_screen() {
    char* screen = (char*) VIDEO_ADDRESS;
    for (int i = 0; i < MAX_ROWS * MAX_COLS; i++) {
        screen[i*2] = ' ';
        screen[i*2+1] = WHITE_ON_BLACK;
    }
    update_cursor(0, 0);
}

void kprint_at(char* message, int col, int row) {
    char* screen = (char*) VIDEO_ADDRESS;
    int i = 0;
    while (message[i] != 0) {
        int offset = (row * MAX_COLS + col) * 2;
        screen[offset] = message[i];
        screen[offset+1] = WHITE_ON_BLACK;
        i++;
        col++;
        if (col >= MAX_COLS) { col = 0; row++; }
    }
    update_cursor(col, row);
}

void kprint(char* message) {
    // Упрощенно: печатаем в начало, пока не сделаем менеджер памяти
    kprint_at(message, 0, 0);
}