#ifndef BMP_H
#define BMP_H

#include <stdint.h>

// Упакованные структуры, чтобы компилятор не вставил лишних байт
typedef struct {
    uint16_t type;              // Должно быть 0x4D42 ('BM')
    uint32_t size;              // Полный размер файла
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;            // Отступ от начала файла до массива пикселей
} __attribute__((packed)) bmp_file_header_t;

typedef struct {
    uint32_t size;
    int32_t  width;
    int32_t  height;
    uint16_t planes;
    uint16_t bit_count;         // Нас интересует 24 бита
    uint32_t compression;
    uint32_t size_image;
    int32_t  x_pixels_per_meter;
    int32_t  y_pixels_per_meter;
    uint32_t colors_used;
    uint32_t colors_important;
} __attribute__((packed)) bmp_info_header_t;

void draw_bmp(const uint8_t* data, int x, int y);

#endif