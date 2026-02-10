#include "memory.h"

static block_header_t* heap_start = NULL;

size_t align_size(size_t size) {
    return (size + HEAP_ALIGNMENT - 1) & ~(HEAP_ALIGNMENT - 1);
}

void init_heap(uint64_t start_addr, size_t size) {
    uint64_t aligned_start = align_size(start_addr);
    size -= (aligned_start - start_addr);

    heap_start = (block_header_t*)aligned_start;
    heap_start->size = size;
    heap_start->free = 1;
    heap_start->next = NULL;
}

void* kmalloc(size_t size) {
    if (size == 0) return NULL;

    size_t total_needed = align_size(size + sizeof(block_header_t));
    block_header_t* current = heap_start;

    while (current) {
        if (current->free && current->size >= total_needed) {
            // Если места много, отрезаем кусок
            if (current->size >= total_needed + sizeof(block_header_t) + 16) {
                block_header_t* next_node = (block_header_t*)((uint8_t*)current + total_needed);
                next_node->size = current->size - total_needed;
                next_node->free = 1;
                next_node->next = current->next;

                current->size = total_needed;
                current->next = next_node;
            }
            
            current->free = 0;
            return (void*)((uint8_t*)current + sizeof(block_header_t));
        }
        current = current->next;
    }
    return NULL; 
}

void kfree(void* ptr) {
    if (!ptr) return;
    block_header_t* header = (block_header_t*)((uint8_t*)ptr - sizeof(block_header_t));
    header->free = 1;
    
    // Слияние (простая проверка следующего блока)
    block_header_t* curr = heap_start;
    while (curr && curr->next) {
        if (curr->free && curr->next->free) {
            curr->size += curr->next->size;
            curr->next = curr->next->next;
        }
        curr = curr->next;
    }
}