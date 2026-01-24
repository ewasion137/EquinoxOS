#include "memory.h"
#include <stdint.h>
// Начинаем выделять память с 2-го мегабайта (0x200000)
// Это безопасная зона, там точно нет ядра или видеопамяти
uint32_t free_mem_addr = 0x200000;

void init_memory() {
    // Тут в будущем будет проверка общего объема ОЗУ
}

void* kmalloc(uint32_t size) {
    uint32_t tmp = free_mem_addr;
    free_mem_addr += size;
    return (void*)tmp;
}