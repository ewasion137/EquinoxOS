#include "screen.h"
#include "../../io/io.h"
#include <stdarg.h>

// Указываем, что эти переменные объявлены в kernel.c
extern int current_col;
extern int current_row;

/* Вспомогательная функция для перевода числа в строку */
void itoa(int n, char str[]) {
    int i, sign;
    if ((sign = n) < 0) n = -n;
    i = 0;
    do {
        str[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    if (sign < 0) str[i++] = '-';
    str[i] = '\0';

    for (int j = 0, k = i-1; j < k; j++, k--) {
        char temp = str[j];
        str[j] = str[k];
        str[k] = temp;
    }
}

/* Сдвиг экрана вверх */
void scroll_screen() {
    char* video_mem = (char*) VIDEO_ADDRESS;
    for (int i = 0; i < (MAX_ROWS - 1) * MAX_COLS * 2; i++) {
        video_mem[i] = video_mem[i + MAX_COLS * 2];
    }
    int last_row_start = (MAX_ROWS - 1) * MAX_COLS * 2;
    for (int i = last_row_start; i < MAX_ROWS * MAX_COLS * 2; i += 2) {
        video_mem[i] = ' ';
        video_mem[i+1] = WHITE_ON_BLACK;
    }
}

/* Обновление курсора */
void update_cursor(int x, int y) {
    unsigned short pos = y * MAX_COLS + x;
    outb(REG_SCREEN_CTRL, 0x0E);
    outb(REG_SCREEN_DATA, (unsigned char)((pos >> 8) & 0xFF));
    outb(REG_SCREEN_CTRL, 0x0F);
    outb(REG_SCREEN_DATA, (unsigned char)(pos & 0xFF));
}

void clear_screen() {
    char* screen = (char*) VIDEO_ADDRESS;
    for (int i = 0; i < MAX_ROWS * MAX_COLS; i++) {
        screen[i*2] = ' ';
        screen[i*2+1] = WHITE_ON_BLACK;
    }
    current_col = 0;
    current_row = 0;
    update_cursor(0, 0);
}

/* Печать с автоматическим обновлением глобальных координат */
void kprint_at(char* message, int col, int row) {
    char* screen = (char*) VIDEO_ADDRESS;
    int i = 0;
    while (message[i] != 0) {
        if (message[i] == '\n') {
            col = 0;
            row++;
        } else {
            if (col >= MAX_COLS) {
                col = 0;
                row++;
            }
            if (row >= MAX_ROWS) {
                scroll_screen();
                row = MAX_ROWS - 1;
            }
            int offset = (row * MAX_COLS + col) * 2;
            screen[offset] = message[i];
            screen[offset+1] = WHITE_ON_BLACK;
            col++;
        }
        i++;
    }
    // Сохраняем новые координаты обратно в ядро
    current_col = col;
    current_row = row;
    update_cursor(current_col, current_row);
}

void kprint(char* message) {
    kprint_at(message, current_col, current_row);
}

void kprintf(char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[32];
    for (char* p = format; *p != '\0'; p++) {
        if (*p != '%') {
            char str[2] = {*p, 0};
            kprint(str);
            continue;
        }
        p++;
        switch (*p) {
            case 'd': {
                int d = va_arg(args, int);
                itoa(d, buffer);
                kprint(buffer);
                break;
            }
            case 's': {
                char* s = va_arg(args, char*);
                kprint(s);
                break;
            }
            default: kprint("?"); break;
        }
    }
    va_end(args);
}