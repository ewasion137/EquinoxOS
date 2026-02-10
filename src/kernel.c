// kernel.c
#include "drivers/vga/vesa.h"
#include "boot/limine/limine.h"
#include <stdint.h>
#include <stddef.h>
#include "system/pic.h"        // <-- ДОБАВЬ ЭТО
#include "system/idt.h"
#include "system/memory.h"
#include "drivers/mouse/mouse.h"
#include "drivers/vga/equinox_img.h" // Тот самый файл с гигантским массивом
#include "drivers/vga/bmp.h"
#define HHDM_OFFSET 0xFFFFFFFF80000000 

static uint8_t kernel_heap_area[2 * 1024 * 1024];

static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST_ID,
    .revision = 0
};

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

static uint8_t cursor_shape[16][16] = {
    {2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {2,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0},
    {2,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0},
    {2,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0},
    {2,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0},
    {2,1,1,1,1,1,1,2,0,0,0,0,0,0,0,0},
    {2,1,1,1,1,1,1,1,2,0,0,0,0,0,0,0},
    {2,1,1,1,1,2,2,2,2,2,0,0,0,0,0,0},
    {2,1,1,2,1,2,0,0,0,0,0,0,0,0,0,0},
    {2,1,2,0,2,1,2,0,0,0,0,0,0,0,0,0},
    {2,2,0,0,2,1,2,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,2,1,2,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0}
};

void draw_cursor(int x, int y) {
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            if (cursor_shape[i][j] == 1) put_pixel(x + j, y + i, 0xFFFFFF);
            if (cursor_shape[i][j] == 2) put_pixel(x + j, y + i, 0x000000);
        }
    }
}


void keyboard_callback() {
    // Пока просто пустая функция, чтобы не падало
    // Когда починим экран, вернем сюда рисование символов через vesa_draw_char
}

void kmain(void) {
    // 1. Память - фундамент
    init_heap((uintptr_t)kernel_heap_area, sizeof(kernel_heap_area));

    if (framebuffer_request.response == NULL || 
        framebuffer_request.response->framebuffer_count < 1) {
        while(1) { __asm__("hlt"); }
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    init_vesa((uint64_t)fb->address, fb->width, fb->height, fb->pitch);

    // 2. Прерывания и мышь
    __asm__("cli");
    init_idt();
    pic_remap();
    draw_cursor(mouse_x, mouse_y);
    init_mouse();
    __asm__("sti");

    // 3. Отрисовка интерфейса (ОДИН РАЗ)
    draw_background();
    if (module_request.response == NULL || module_request.response->module_count < 1) {
        vesa_draw_string("Error: Logo module not found!", 10, 10, 0xFF0000);
    } else {
        // Берем первый (и единственный) модуль
        struct limine_file *module = module_request.response->modules[0];
        draw_bmp((uint8_t*)module->address, 300, 200);
    }
    vesa_draw_string("EquinoxOS - Graphics Mode", 110, 110, 0xFFFFFF);

    // 4. ТЕСТ ПАМЯТИ (теперь мы его увидим)
    void* test = kmalloc(1024);
    if (test) {
        vesa_draw_string("MALLOC WORKING!", 110, 130, 0x00FF00); // Зеленым
        kfree(test);
    } else {
        vesa_draw_string("MALLOC FAILED!", 110, 130, 0xFF0000); // Красным
    }

    // 5. Подготовка буфера курсора
    uint32_t* bg_buffer = (uint32_t*)kmalloc(16 * 16 * sizeof(uint32_t));
    if (!bg_buffer) { while(1); }

    int32_t old_mouse_x = mouse_x;
    int32_t old_mouse_y = mouse_y;

    // ПЕРВОЕ сохранение фона
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            uint32_t* pixel_ptr = (uint32_t*)(fb_base_addr + ((old_mouse_y + y) * screen_pitch) + ((old_mouse_x + x) * 4));
            bg_buffer[y * 16 + x] = *pixel_ptr;
        }
    }
    draw_cursor(old_mouse_x, old_mouse_y); // Рисуем самый первый раз!

    while(1) {
        if (mouse_x != old_mouse_x || mouse_y != old_mouse_y) {
            // 1. Стираем старый
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    put_pixel(old_mouse_x + x, old_mouse_y + y, bg_buffer[y * 16 + x]);
                }
            }
            // 2. Сохраняем новый фон
            old_mouse_x = mouse_x;
            old_mouse_y = mouse_y;
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    uint32_t* pixel_ptr = (uint32_t*)(fb_base_addr + ((old_mouse_y + y) * screen_pitch) + ((old_mouse_x + x) * 4));
                    bg_buffer[y * 16 + x] = *pixel_ptr;
                }
            }
            // 3. Рисуем курсор
            draw_cursor(old_mouse_x, old_mouse_y);
        }
        __asm__("hlt");
    }
}