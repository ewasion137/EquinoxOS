// kernel.c
#include "drivers/vga/vesa.h"
#include "boot/limine/limine.h"
#include <stdint.h>
#include <stddef.h>
// #include "system/pic.h"        // <-- ДОБАВЬ ЭТО
#include "system/idt.h"
#include "drivers/mouse/mouse.h"

// Запрос к загрузчику. Limine сам заполнит эту структуру.
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

void keyboard_callback() {
    // Пока просто пустая функция, чтобы не падало
    // Когда починим экран, вернем сюда рисование символов через vesa_draw_char
}

void kmain(void) {
    // 1. Проверяем, что загрузчик ответил
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        // Если экрана нет — стоим насмерть
        while(1) { __asm__ __volatile__("hlt"); }
    }


    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    init_vesa((uint64_t)fb->address, (uint32_t)fb->width, (uint32_t)fb->height, (uint32_t)fb->pitch);

    // --- ПРАВИЛЬНЫЙ ПОРЯДОК ИНИЦИАЛИЗАЦИИ ---
    // pic_remap(); // <-- ВЫЗЫВАЙ ПЕРЕД IDT
    init_idt();  // Инициализирует IDT и включает прерывания STI
    init_mouse(); // Инициализирует PS/2 контроллер мыши

    // 3. Передаем адрес, ширину, высоту и pitch в драйвер VESA
    // Саму переменную framebuffer мы тут не храним, она спрятана внутри init_vesa
    init_vesa((uint64_t)fb->address, (uint32_t)fb->width, (uint32_t)fb->height, (uint32_t)fb->pitch);

    // 4. Рисуем
    draw_background(); 
    
    // Окно
    draw_transparent_rect(100, 100, 600, 400, 0xFFFFFF, 150);
    draw_rect(100, 100, 600, 30, 0x0055AA);
    
    vesa_draw_string("EquinoxOS - Graphics Works!", 110, 110, 0xFFFFFF);
    vesa_draw_string("No more stripes please...", 120, 150, 0x000000);

    volatile int32_t old_mouse_x = 0; // <-- ОБЯЗАТЕЛЬНО volatile
    volatile int32_t old_mouse_y = 0; // <-- ОБЯЗАТЕЛЬНО volatile

    // Буфер для сохранения фона под курсором (16x16 пикселей)
    uint32_t bg_buffer[16 * 16];

    while(1) {
        // 1. Восстанавливаем фон там, где курсор был в прошлом кадре
        // Проверка на корректность координат
        if (old_mouse_x >= 0 && old_mouse_x < (int32_t)screen_width &&
            old_mouse_y >= 0 && old_mouse_y < (int32_t)screen_height) {
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    put_pixel(old_mouse_x + x, old_mouse_y + y, bg_buffer[y * 16 + x]);
                }
            }
        }
        
        // 2. Сохраняем фон там, где курсор будет в этом кадре
        // Проверка на корректность координат
        if (mouse_x >= 0 && mouse_x < (int32_t)screen_width &&
            mouse_y >= 0 && mouse_y < (int32_t)screen_height) {
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    uint32_t* pixel_ptr = (uint32_t*)(fb_base_addr + ((mouse_y + y) * screen_pitch) + ((mouse_x + x) * 4));
                    // Проверка, что мы не читаем за пределами экрана
                    if ((mouse_x + x) < (int32_t)screen_width && (mouse_y + y) < (int32_t)screen_height) {
                        bg_buffer[y * 16 + x] = *pixel_ptr;
                    }
                }
            }
        }
        
        // 3. Рисуем сам курсор (простой белый квадрат 10x10)
        draw_rect(mouse_x, mouse_y, 10, 10, 0xFFFFFF);
        if (mouse_left_button) { 
             draw_rect(mouse_x, mouse_y, 10, 10, 0xFF0000);
        }

        // 4. Обновляем старые координаты
        old_mouse_x = mouse_x;
        old_mouse_y = mouse_y;

        __asm__ __volatile__("hlt");
    }
}
