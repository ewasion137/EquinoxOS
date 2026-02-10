// src/system/string.c
#include "string.h" // Включаем свой заголовочный файл

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}