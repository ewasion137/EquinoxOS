#ifndef FS_H
#define FS_H

#include <stdint.h>

#define MAX_FILES 16 // Пока хватит 16 файлов на диск

typedef struct {
    char name[20];
    uint32_t size;
    uint32_t start_lba;
    uint32_t unused;
} __attribute__((packed)) file_entry_t;

void init_fs();
void list_files();
void create_file(char* name, char* content);
void read_file(char* name);

#endif