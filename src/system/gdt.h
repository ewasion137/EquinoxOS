#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// Структура записи в GDT
struct gdt_entry_struct {
    uint16_t limit_low;           // Нижние 16 бит лимита
    uint16_t base_low;            // Нижние 16 бит базы
    uint8_t  base_middle;         // Следующие 8 бит базы
    uint8_t  access;              // Флаги доступа
    uint8_t  granularity;
    uint8_t  base_high;           // Последние 8 бит базы
} __attribute__((packed));

typedef struct gdt_entry_struct gdt_entry_t;

// Структура указателя на GDT (то, что съест процессор)
struct gdt_ptr_struct {
    uint16_t limit;               // Размер таблицы
    uint64_t base;                // Адрес начала таблицы
} __attribute__((packed));

typedef struct gdt_ptr_struct gdt_ptr_t;

// Инициализация
void init_gdt();

#endif