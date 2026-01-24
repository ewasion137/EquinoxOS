#ifndef ATA_H
#define ATA_H

#include <stdint.h>

// Чтение одного сектора (512 байт) с диска
void read_sectors_ata_pio(uint32_t target_address, uint32_t LBA, uint8_t sector_count);
void write_sectors_ata_pio(uint32_t LBA, uint8_t sector_count, uint16_t* buffer);

#endif