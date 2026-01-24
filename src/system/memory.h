#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

void* kmalloc(uint32_t size);
void init_memory();

#endif