// fs.c
#include "fs.h"
#include "../drivers/disk/ata.h"
#include "../system/memory.h"
#include "../drivers/vga/vesa.h" // Добавляем VESA для вывода

void list_files() {
    // Выделяем память под сектор. 
    // ВАЖНО: убедись, что твой kmalloc работает в 64-битном режиме!
    file_entry_t* dir = (file_entry_t*)kmalloc(512);
    if (!dir) return;

    read_sectors_ata_pio((uint64_t)dir, 1, 1); 

    int y_pos = 180; // Начальная позиция для списка файлов на экране
    int found = 0;

    vesa_draw_string("Files on disk:", 120, y_pos, 0x00FF00);
    y_pos += 15;

    for (int i = 0; i < MAX_FILES; i++) {
        if (dir[i].name[0] != 0) {
            vesa_draw_string(dir[i].name, 130, y_pos, 0xFFFFFF);
            y_pos += 12;
            found = 1;
        }
    }

    if (!found) {
        vesa_draw_string("No files found.", 130, y_pos, 0xAAAAAA);
    }
    
    // В идеале тут должен быть kfree(dir), если он у тебя написан
}

void create_file(char* name, char* content) {
    file_entry_t* dir = (file_entry_t*)kmalloc(512);
    if (!dir) return;
    
    read_sectors_ata_pio((uint64_t)dir, 1, 1);

    for (int i = 0; i < MAX_FILES; i++) {
        if (dir[i].name[0] == 0) {
            // Копируем имя
            for(int j=0; j<20 && name[j] != '\0'; j++) {
                dir[i].name[j] = name[j];
            }
            dir[i].size = 512;
            dir[i].start_lba = 10 + i;

            // Готовим данные
            uint16_t* data = (uint16_t*)kmalloc(512);
            for(int j=0; j<256; j++) data[j] = 0;
            
            char* c_ptr = (char*)data;
            for(int j=0; j < 512 && content[j] != '\0'; j++) {
                c_ptr[j] = content[j];
            }
            
            // Пишем на диск
            write_sectors_ata_pio(dir[i].start_lba, 1, data);
            write_sectors_ata_pio(1, 1, (uint16_t*)dir);
            
            vesa_draw_string("File created successfully!", 120, 350, 0x00FF00);
            return;
        }
    }
}