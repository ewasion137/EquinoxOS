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
    int y_debug_offset = 180; // Начальная Y-координата для вывода дебага
    
    mouse_flush_buffer(); 
    vesa_draw_string("Mouse Init: Flushed buffer.", 120, y_debug_offset, 0xFFFFFF); y_debug_offset += 12;

    // 1. Отключаем мышь
    mouse_wait_input(); outb(PS2_CMD_PORT, 0xA7); 
    vesa_draw_string("Mouse Init: Disabled aux device (0xA7).", 120, y_debug_offset, 0xFFFFFF); y_debug_offset += 12;

    // 2. Читаем Controller Command Byte (CCB)
    mouse_wait_input(); outb(PS2_CMD_PORT, 0x20); // Команда "прочитать CCB"
    uint8_t ccb = mouse_read();
    vesa_draw_string_hex("Mouse Init: CCB before: 0x", 120, y_debug_offset, ccb, 0xFFFFFF); y_debug_offset += 12;

    // 3. Изменяем CCB: Включаем IRQ12 (бит 1), Включаем часы мыши (бит 5)
    ccb |= 0x02;  // Включаем IRQ12
    ccb &= ~0x20; // Очищаем бит 5 (т.е. включаем часы мыши)
    // НЕ включаем translation (бит 6), пока не убедимся, что все остальное работает
    
    // 4. Записываем измененный CCB
    mouse_wait_input(); outb(PS2_CMD_PORT, 0x60); // Команда "записать CCB"
    mouse_wait_input(); outb(PS2_DATA_PORT, ccb);
    vesa_draw_string_hex("Mouse Init: CCB after:  0x", 120, y_debug_offset, ccb, 0xFFFFFF); y_debug_offset += 12;

    // 5. Включаем мышь (Enable Auxiliary Device)
    mouse_wait_input(); outb(PS2_CMD_PORT, 0xA8);
    vesa_draw_string("Mouse Init: Enabled aux device (0xA8).", 120, y_debug_offset, 0xFFFFFF); y_debug_offset += 12;

    // 6. Проверка PS/2 контроллера (Self-Test)
    mouse_wait_input(); outb(PS2_CMD_PORT, 0xAA); // Команда "Self-Test" (Должен вернуть 0x55)
    uint8_t self_test_response = mouse_read(); 
    vesa_draw_string_hex("Mouse Init: Self-Test (0xAA) response: 0x", 120, y_debug_offset, self_test_response, 0xFFFFFF); y_debug_offset += 12;
    // Если self_test_response НЕ 0x55, это ОГРОМНАЯ ПРОБЛЕМА С КОНТРОЛЛЕРОМ!

    // 7. Активируем порт для мыши
    mouse_wait_input(); outb(PS2_CMD_PORT, 0xAE); // Enable PS/2 auxiliary device port
    vesa_draw_string("Mouse Init: Enabled aux port (0xAE).", 120, y_debug_offset, 0xFFFFFF); y_debug_offset += 12;

    // 8. Сброс мыши и установка режима
    mouse_write(0xFF); // Reset mouse
    uint8_t ack_reset = mouse_read(); // Ожидаем ACK (0xFA)
    uint8_t id_reset = mouse_read();  // Ожидаем ID (0xAA)
    vesa_draw_string_hex("Mouse Init: Reset (0xFF) ACK: 0x", 120, y_debug_offset, ack_reset, 0xFFFFFF); y_debug_offset += 12;
    vesa_draw_string_hex("Mouse Init: Reset (0xFF) ID:  0x", 120, y_debug_offset, id_reset, 0xFFFFFF); y_debug_offset += 12;
    // Оба должны быть 0xFA и 0xAA соответственно. Если нет - мышь не отвечает.

    mouse_write(0xF6); // Set defaults
    uint8_t ack_defaults = mouse_read();
    vesa_draw_string_hex("Mouse Init: Set defaults (0xF6) ACK: 0x", 120, y_debug_offset, ack_defaults, 0xFFFFFF); y_debug_offset += 12;

    mouse_write(0xF3); // Set sample rate
    uint8_t ack_samplerate_cmd = mouse_read();
    mouse_write(200);  // 200 сэмплов в секунду
    uint8_t ack_samplerate_data = mouse_read();
    vesa_draw_string_hex("Mouse Init: Sample rate (0xF3) ACK_CMD: 0x", 120, y_debug_offset, ack_samplerate_cmd, 0xFFFFFF); y_debug_offset += 12;
    vesa_draw_string_hex("Mouse Init: Sample rate (200) ACK_DATA: 0x", 120, y_debug_offset, ack_samplerate_data, 0xFFFFFF); y_debug_offset += 12;

    mouse_write(0xF4); // Enable data reporting (включить передачу данных)
    uint8_t ack_enable = mouse_read();
    vesa_draw_string_hex("Mouse Init: Enable data (0xF4) ACK: 0x", 120, y_debug_offset, ack_enable, 0xFFFFFF); y_debug_offset += 12;

    vesa_draw_string("Mouse Init: DONE. Now move the mouse!", 120, y_debug_offset, 0x00FF00); y_debug_offset += 12;
}