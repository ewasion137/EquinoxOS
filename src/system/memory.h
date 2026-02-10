// src/system/memory.h
#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h> // Для size_t

// Макрос для выравнивания адресов
// HEAP_ALIGNMENT - это размер страницы или кэш-линии (например, 4 или 8 байт)
#define HEAP_ALIGNMENT 8 

// Структура для заголовка каждого блока памяти в свободном списке
// (как свободного, так и занятого)
typedef struct block_header {
    size_t size;                // Размер блока (включая заголовок)
    struct block_header* next;  // Указатель на следующий свободный блок (только если блок свободен)
    uint8_t  free;              // Флаг: 1 = свободен, 0 = занят
} block_header_t;

void init_heap(uint64_t heap_start_addr, size_t heap_size);
void* kmalloc(size_t size);
void kfree(void* ptr);

#endif