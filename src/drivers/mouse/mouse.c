#include "../../io/io.h"
#include "../../system/idt.h"
#include "../vga/vesa.h"
#include "mouse.h"

int mouse_x = 400; // Стартуем в центре
int mouse_y = 300;
uint8_t mouse_cycle = 0;
int8_t mouse_byte[3];

// Команды контроллеру 8042
void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        while (timeout--) { if ((inb(0x64) & 1) == 1) return; }
    } else {
        while (timeout--) { if ((inb(0x64) & 2) == 0) return; }
    }
}

void mouse_write(uint8_t a) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, a);
}

uint8_t mouse_read() {
    mouse_wait(0);
    return inb(0x60);
}

void init_mouse() {
    uint8_t status;

    mouse_wait(1);
    outb(0x64, 0xA8); // Включить мышь

    mouse_wait(1);
    outb(0x64, 0x20); // Получить статус
    mouse_wait(0);
    status = (inb(0x60) | 2);
    mouse_wait(1);
    outb(0x64, 0x60); // Записать статус
    mouse_wait(1);
    outb(0x60, status);

    mouse_write(0xF4); // Включить передачу данных
    mouse_read();      // Подтверждение (ACK)
}

void mouse_handler() {
    uint8_t status = inb(0x64);
    
    // Проверяем, есть ли данные и от мыши ли они (бит 5 должен быть 1)
    if (!(status & 0x01) || !(status & 0x20)) {
        // Если это не мышь, просто выходим, но подтверждаем прерывание
        outb(0x20, 0x20);
        return; 
    }

    mouse_byte[mouse_cycle++] = inb(0x60);

    // СИНХРОНИЗАЦИЯ: Первый байт пакета ОБЯЗАН иметь 3-й бит равным 1.
    // Если мы получили первый байт и там 3-й бит = 0, значит мы "потерялись".
    if (mouse_cycle == 1 && !(mouse_byte[0] & 0x08)) {
        mouse_cycle = 0; // Сбрасываем цикл и ждем следующий байт
        outb(0x20, 0x20);
        return;
    }

    if (mouse_cycle == 3) {
        mouse_cycle = 0;

        // Расчет X (используем знаковые переменные)
        int x_move = (int)mouse_byte[1];
        if (mouse_byte[0] & 0x10) { // Если 4-й бит (X sign) = 1
            x_move -= 256;
        }

        // Расчет Y (используем знаковые переменные)
        int y_move = (int)mouse_byte[2];
        if (mouse_byte[0] & 0x20) { // Если 5-й бит (Y sign) = 1
            y_move -= 256;
        }

        // Применяем движение
        // Делим на 2 или 3, если мышь слишком быстрая
        mouse_x += x_move; 
        mouse_y -= y_move; // В VESA инвертируем, чтобы движение вниз увеличивало Y

        // ОГРАНИЧИТЕЛИ (Clamping)
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x > 795) mouse_x = 795;
        if (mouse_y > 595) mouse_y = 595;
    }

    // Подтверждаем прерывание обоим контроллерам
    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}