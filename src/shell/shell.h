#ifndef SHELL_H
#define SHELL_H

// Основные функции шелла
void execute_command(char* input);
void shell_clear();
void shell_beep();

// ДОБАВЬ ЭТИ ДВЕ СТРОЧКИ:
void shell_save(char* arg);
void shell_load();
void read_file(char* name);

#endif