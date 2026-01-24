#include "ata.h"
#include "../../io/io.h"

// 0xE0 для Master (диск 0), 0xF0 для Slave (диск 1)
#define ATA_MASTER 0xE0
#define ATA_SLAVE  0xF0

static void ata_io_wait() {
    for(int i = 0; i < 4; i++) inb(0x1F7);
}

static void ata_wait_bsy() {
    while (inb(0x1F7) & 0x80);
}

static void ata_wait_drq() {
    while (!(inb(0x1F7) & 0x08));
}

void read_sectors_ata_pio(uint32_t target_address, uint32_t LBA, uint8_t sector_count) {
    ata_wait_bsy();
    // ИСПОЛЬЗУЕМ 0xF0 ТАК КАК fs.img ЭТО SLAVE (index=1)
    outb(0x1F6, ATA_SLAVE | ((LBA >> 24) & 0x0F));
    outb(0x1F2, sector_count);
    outb(0x1F3, (uint8_t)LBA);
    outb(0x1F4, (uint8_t)(LBA >> 8));
    outb(0x1F5, (uint8_t)(LBA >> 16));
    outb(0x1F7, 0x20); // Command: Read

    uint16_t* target = (uint16_t*)target_address;

    for (int j = 0; j < sector_count; j++) {
        ata_wait_bsy();
        ata_wait_drq();
        for (int i = 0; i < 256; i++) {
            target[i] = inw(0x1F0);
        }
        target += 256;
    }
}

void write_sectors_ata_pio(uint32_t LBA, uint8_t sector_count, uint16_t* buffer) {
    ata_wait_bsy();
    outb(0x1F6, ATA_SLAVE | ((LBA >> 24) & 0x0F));
    outb(0x1F2, sector_count);
    outb(0x1F3, (uint8_t)LBA);
    outb(0x1F4, (uint8_t)(LBA >> 8));
    outb(0x1F5, (uint8_t)(LBA >> 16));
    outb(0x1F7, 0x30); // Command: Write

    for (int j = 0; j < sector_count; j++) {
        ata_wait_bsy();
        ata_wait_drq();
        for (int i = 0; i < 256; i++) {
            outw(0x1F0, buffer[i]);
        }
    }
    
    // ОТПРАВЛЯЕМ КОМАНДУ СБРОСА КЭША
    outb(0x1F7, 0xE7); 
    ata_wait_bsy();
}