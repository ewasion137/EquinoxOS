#include <stdint.h>
#include "shell.h"
#include "../drivers/screen/screen.h"
#include "../io/io.h"
#include "../system/timer.h"

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
    if (str_compare(input, "help") == 0) {
        shell_help();
    } else if (str_compare(input, "clear") == 0) {
        shell_clear();
    } else if (str_compare(input, "beep") == 0) {
        shell_beep();
    } else if (input[0] == '\0') {
        // Пропускаем пустой ввод
    } else if (str_compare(input, "uptime") == 0) {
        shell_uptime();
    } else {
        kprintf("Unknown command: %s\n", input);
    }
}

void shell_help() {
    kprint("Commands: help, clear, beep\n");
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