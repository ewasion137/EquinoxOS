// shell.c
#include <stdint.h>
#include "shell.h"
#include "../drivers/screen/screen.h"
#include "../io/io.h"
#include "../system/timer.h"
#include "../drivers/disk/ata.h"
#include "../system/memory.h"
#include "../fs/fs.h"

extern uint32_t tick;

// Сравнение строк
int str_compare(char* s1, char* s2) {
    int i = 0;
    while (s1[i] == s2[i]) {
        if (s1[i] == '\0') return 0;
        i++;
    }
    return s1[i] - s2[i];
}

void str_copy(char* dest, char* src) {
    int i = 0;
    while ((dest[i] = src[i]) != '\0') i++;
}

// Функции звука
void play_sound(uint32_t nFrequence) {
    uint32_t Div;
    uint8_t tmp;
    Div = 1193180 / nFrequence;
    outb(0x43, 0xB6);
    outb(0x42, (uint8_t) (Div) );
    outb(0x42, (uint8_t) (Div >> 8));
    tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}

void stop_sound() {
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}

// Менеджер команд
void execute_command(char* input) {
    char cmd[32];
    char arg1[32];
    char arg2[128];

    // Очищаем буферы
    for(int i=0; i<32; i++) { cmd[i]=0; arg1[i]=0; }
    for(int i=0; i<128; i++) arg2[i]=0;

    int i = 0, j = 0;

    // 1. Извлекаем команду (до первого пробела)
    while (input[i] != ' ' && input[i] != '\0') {
        cmd[j++] = input[i++];
    }
    cmd[j] = '\0';

    // 2. Извлекаем первый аргумент (имя файла)
    if (input[i] == ' ') {
        i++; j = 0;
        while (input[i] != ' ' && input[i] != '\0') {
            arg1[j++] = input[i++];
        }
        arg1[j] = '\0';
    }

    // 3. Извлекаем остаток (контент для файла)
    if (input[i] == ' ') {
        i++; j = 0;
        while (input[i] != '\0') {
            arg2[j++] = input[i++];
        }
        arg2[j] = '\0';
    }

    // --- МЕНЕДЖЕР КОМАНД ---
    if (str_compare(cmd, "help") == 0) {
        kprint("Commands: ls, touch [name] [text], cat [name], clear, beep\n");
    } 
    else if (str_compare(cmd, "ls") == 0) {
        list_files();
    } 
    else if (str_compare(cmd, "touch") == 0) {
        if (arg1[0] == '\0') kprint("Usage: touch [name] [text]\n");
        else create_file(arg1, arg2);
    } 
    else if (str_compare(cmd, "cat") == 0) {
        if (arg1[0] == '\0') kprint("Usage: cat [name]\n");
        else read_file(arg1);
    } 
    else if (str_compare(cmd, "clear") == 0) {
        clear_screen();
    } 
    else if (input[0] != '\0') {
        kprintf("Unknown command: %s\n", cmd);
    }
}

void shell_clear() {
    clear_screen();
}

void shell_beep() {
    play_sound(1000);
    for(int i = 0; i < 20000000; i++) { __asm__("nop"); }
    stop_sound();
}

void shell_uptime() {
    uint32_t total_seconds = tick / 100;
    uint32_t minutes = total_seconds / 60;
    uint32_t seconds = total_seconds % 60;

    kprintf("System Uptime: %d min, %d sec (Total ticks: %d)\n", minutes, seconds, tick);
}

void shell_save(char* text) {
    uint16_t* buffer = (uint16_t*)kmalloc(512); // Выделяем 512 байт (1 сектор)
    
    // Обнуляем буфер
    for(int i=0; i<256; i++) buffer[i] = 0;
    
    // Копируем текст в буфер (приводим char* к uint16_t*)
    char* char_ptr = (char*)buffer;
    str_copy(char_ptr, text);

    // Пишем в 100-й сектор
    write_sectors_ata_pio(100, 1, buffer);
    kprintf("Text saved to Sector 100: %s\n", text);
}

void shell_load() {
    // Выделяем память с запасом (+1 байт для нуля)
    char* buffer = (char*)kmalloc(512 + 1);
    
    for(int i=0; i<512; i++) buffer[i] = 0; // Чистим буфер заранее

    read_sectors_ata_pio((uint32_t)buffer, 100, 1);
    
    buffer[512] = '\0'; // Гарантируем конец строки

    if (buffer[0] == 0) {
        kprint("Sector 100 is empty.\n");
    } else {
        kprintf("Data loaded: %s\n", buffer);
    }
}

void read_file(char* name) {
    file_entry_t* dir = (file_entry_t*)kmalloc(512);
    read_sectors_ata_pio((uint32_t)dir, 1, 1);

    for (int i = 0; i < MAX_FILES; i++) {
        // Сравниваем имя (используй свою функцию сравнения строк)
        if (str_compare(dir[i].name, name) == 0) {
            char* buffer = (char*)kmalloc(512);
            read_sectors_ata_pio((uint32_t)buffer, dir[i].start_lba, 1);
            kprintf("File content: %s\n", buffer);
            return;
        }
    }
    kprintf("File '%s' not found.\n", name);
}