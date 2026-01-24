#include "keyboard.h"

// Состояние шифта
static int shift_pressed = 0;

// Обычная раскладка
const char ascii_table[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

// Раскладка с зажатым Шифтом
const char ascii_table_shift[] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};

char get_ascii_char(unsigned char scancode) {
    // Проверяем нажатие Шифтов (Левый: 0x2A, Правый: 0x36)
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return 0;
    }
    // Проверяем отпускание Шифтов (Код нажатия + 0x80)
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        return 0;
    }

    // Если это отпускание любой другой клавиши — игнорим
    if (scancode & 0x80) return 0;

    if (scancode > 58) return 0;

    return shift_pressed ? ascii_table_shift[scancode] : ascii_table[scancode];
}