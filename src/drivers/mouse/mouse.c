#include "mouse.h"
#include "../../io/io.h"
#include "../vga/vesa.h" // Для screen_width, screen_height и put_pixel

// Порты PS/2 контроллера
#define PS2_DATA_PORT   0x60
#define PS2_CMD_PORT    0x64

// Глобальные переменные (ОБЯЗАТЕЛЬНО volatile!)
volatile int32_t mouse_x = 0; // Начальные координаты курсора
volatile int32_t mouse_y = 0;
volatile uint8_t mouse_left_button = 0;
volatile uint8_t mouse_right_button = 0;

static uint8_t mouse_packet[3];
static int mouse_cycle = 0;

// Ждем, пока контроллер будет готов принять команду
void mouse_wait_input() {
    uint32_t timeout = 100000; // Добавим таймаут на случай зависания
    while ((inb(PS2_CMD_PORT) & 2) && timeout--);
}

// Ждем, пока контроллер пришлет данные
void mouse_wait_output() {
    uint32_t timeout = 100000;
    while (!(inb(PS2_CMD_PORT) & 1) && timeout--);
}

// Отправка команды контроллеру (без D4 для команд контроллеру)
void ps2_command_write(uint8_t data) {
    mouse_wait_input();
    outb(PS2_CMD_PORT, data);
}

// Отправка команды мыши (с D4)
void mouse_write(uint8_t data) {
    mouse_wait_input();
    outb(PS2_CMD_PORT, 0xD4); // Говорим контроллеру, что шлем команду мыши
    mouse_wait_input();
    outb(PS2_DATA_PORT, data);
}

// Чтение ответа
uint8_t mouse_read() {
    mouse_wait_output();
    return inb(PS2_DATA_PORT);
}

// Вспомогательная функция для очистки буфера данных PS/2
void mouse_flush_buffer() {
    // Читаем из порта данных, пока бит "Output Buffer Full" не сбросится
    uint32_t timeout = 100000;
    while ((inb(PS2_CMD_PORT) & 1) && timeout--) {
        inb(PS2_DATA_PORT);
    }
}

// Это наш обработчик прерывания (Interrupt Handler)
void mouse_callback() {
    uint8_t status = inb(PS2_CMD_PORT);
    // Проверяем, что данные пришли именно от мыши (Auxiliary Device Output Buffer Full - бит 5)
    if (!(status & 0x20)) return;

    uint8_t data = inb(PS2_DATA_PORT);

    // put_pixel(10, 10, 0xFF00FF); // Дебажный пиксель. Если он есть, прерывание срабатывает.

    switch(mouse_cycle) {
        case 0: // Первый байт пакета (флаги и состояние кнопок)
            // Бит 3 должен быть установлен в 1 для корректной синхронизации пакета
            if (!(data & 0x08)) {
                mouse_cycle = 0; // Некорректный пакет, сбрасываем цикл
                return;
            }
            mouse_packet[0] = data;
            mouse_cycle++;
            break;
        case 1: // Второй байт (смещение по X)
            mouse_packet[1] = data;
            mouse_cycle++;
            break;
        case 2: // Третий байт (смещение по Y)
            mouse_packet[2] = data;
            mouse_cycle = 0; // Сбрасываем цикл для следующего пакета

            // --- Обработка полного пакета ---
            int8_t delta_x = mouse_packet[1];
            int8_t delta_y = mouse_packet[2];

            // Обновляем состояние кнопок
            mouse_left_button = mouse_packet[0] & 0x01;
            mouse_right_button = (mouse_packet[0] >> 1) & 0x01;

            // Обновляем координаты (Y-ось инвертирована для PS/2 мыши)
            mouse_x += delta_x;
            mouse_y -= delta_y;

            // Ограничиваем движение курсора экраном
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_x >= (int32_t)screen_width) mouse_x = screen_width - 1;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_y >= (int32_t)screen_height) mouse_y = screen_height - 1;
            
            // put_pixel(10, 10, 0xFF00FF); // Дебажный пиксель, чтобы убедиться, что сюда дошли
            break;
    }
}

void init_mouse() {
    mouse_flush_buffer(); // Очищаем буфер данных PS/2 от мусора

    // 1. Отключаем мышь (Disable Auxiliary Device)
    mouse_wait_input();
    outb(PS2_CMD_PORT, 0xA7); 

    // 2. Читаем Controller Command Byte (CCB)
    mouse_wait_input();
    outb(PS2_CMD_PORT, 0x20); // Команда "прочитать CCB"
    mouse_wait_output();
    uint8_t ccb = inb(PS2_DATA_PORT);

    // 3. Изменяем CCB: Включаем IRQ12 (бит 1), Включаем часы мыши (бит 5)
    ccb |= 0x02; // Включаем IRQ12
    ccb &= ~0x20; // Очищаем бит 5 (т.е. включаем часы мыши)
    // Опционально: ccb |= 0x40; // Включить PS/2 "Translation" (не всегда нужно)

    // 4. Записываем измененный CCB
    mouse_wait_input();
    outb(PS2_CMD_PORT, 0x60); // Команда "записать CCB"
    mouse_wait_input();
    outb(PS2_DATA_PORT, ccb);

    // 5. Включаем мышь (Enable Auxiliary Device)
    mouse_wait_input();
    outb(PS2_CMD_PORT, 0xA8);

    // 6. Проверка PS/2 контроллера
    mouse_wait_input();
    outb(PS2_CMD_PORT, 0xAA); // Команда "Self-Test" (Должен вернуть 0x55)
    mouse_read(); // Читаем ответ, должен быть 0x55

    mouse_wait_input();
    outb(PS2_CMD_PORT, 0xAE); // Enable PS/2 auxiliary device port

    // 7. Сброс мыши и установка режима
    mouse_write(0xFF); // Reset mouse
    mouse_read();      // Ожидаем ACK (0xFA)
    mouse_read();      // Ожидаем ID (0xAA)

    mouse_write(0xF6); // Set defaults (установить настройки по умолчанию)
    mouse_read();      // Ожидаем ACK (0xFA)

    mouse_write(0xF3); // Set sample rate (установить частоту сэмплирования)
    mouse_read();      // Ожидаем ACK (0xFA)
    mouse_write(200);  // 200 сэмплов в секунду (это достаточно быстро)
    mouse_read();      // Ожидаем ACK (0xFA)

    mouse_write(0xF4); // Enable data reporting (включить передачу данных)
    mouse_read();      // Ожидаем ACK (0xFA)
}