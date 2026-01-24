#ifndef SHELL_H
#define SHELL_H

// Функция для обработки введенной строки
void execute_command(char* input);

// Встроенные команды
void shell_help();
void shell_clear();
void shell_beep();
void shell_uptime();

#endif