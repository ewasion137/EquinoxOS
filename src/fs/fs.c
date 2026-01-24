#include "fs.h"
#include "../drivers/disk/ata.h"
#include "../drivers/screen/screen.h"
#include "../system/memory.h"

// Читаем оглавление с 1-го сектора
void list_files() {
    file_entry_t* dir = (file_entry_t*)kmalloc(512);
    read_sectors_ata_pio((uint32_t)dir, 1, 1); // Читаем 1-й сектор

    kprint("Files on disk:\n");
    int found = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (dir[i].name[0] != 0) {
            kprintf("- %s (%d bytes) at LBA %d\n", dir[i].name, dir[i].size, dir[i].start_lba);
            found = 1;
        }
    }
    if (!found) kprint("No files found.\n");
}

void create_file(char* name, char* content) {
    file_entry_t* dir = (file_entry_t*)kmalloc(512);
    read_sectors_ata_pio((uint32_t)dir, 1, 1);

    // Ищем свободный слот в оглавлении
    for (int i = 0; i < MAX_FILES; i++) {
        if (dir[i].name[0] == 0) {
            // Заполняем инфу о файле
            for(int j=0; j<20; j++) dir[i].name[j] = name[j];
            dir[i].size = 512; // Пока фиксированно 1 сектор
            dir[i].start_lba = 10 + i; // Пишем файлы начиная с 10 сектора

            // Пишем данные файла
            uint16_t* data = (uint16_t*)kmalloc(512);
            for(int j=0; j<256; j++) data[j] = 0;
            char* c_ptr = (char*)data;
            for(int j=0; content[j] != '\0'; j++) c_ptr[j] = content[j];
            
            write_sectors_ata_pio(dir[i].start_lba, 1, data);

            // Сохраняем обновленное оглавление
            write_sectors_ata_pio(1, 1, (uint16_t*)dir);
            kprintf("File '%s' created!\n", name);
            return;
        }
    }
}