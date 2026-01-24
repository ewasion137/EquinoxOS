#ifndef IO_H
#define IO_H
#include <stdint.h>

unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char val);
uint16_t inw(unsigned short port);
void outw(unsigned short port, uint16_t val);

#endif