# makefile

ASM = nasm
CC = gcc
LD = ld
OBJCOPY = objcopy
EMU = qemu-system-x86_64

# Добавили shell.o в список объектов
OBJ = kernel_entry.o kernel.o io.o screen.o keyboard.o gdt.o gdt_flush.o idt.o interrupt.o pic.o shell.o timer.o ata.o memory.o fs.o vga.o font.o

# Добавили путь к папке shell в инклюды
CFLAGS = -ffreestanding -m32 -fno-pie -fno-stack-protector -fno-leading-underscore -Isrc -Isrc/drivers -Isrc/shell

all: os-image.bin run

# 1. Загрузчик
boot.bin: src/boot/boot.asm
	$(ASM) -f bin src/boot/boot.asm -o boot.bin

# 2. Точка входа в ядро
kernel_entry.o: src/boot/kernel_entry.asm
	$(ASM) -f elf32 src/boot/kernel_entry.asm -o kernel_entry.o

# 3. Основное ядро
kernel.o: src/kernel.c
	$(CC) $(CFLAGS) -c src/kernel.c -o kernel.o

# 4. Драйверы и системные модули
io.o: src/io/io.c
	$(CC) $(CFLAGS) -c src/io/io.c -o io.o

screen.o: src/drivers/screen/screen.c
	$(CC) $(CFLAGS) -c src/drivers/screen/screen.c -o screen.o

keyboard.o: src/drivers/keyboard/keyboard.c
	$(CC) $(CFLAGS) -c src/drivers/keyboard/keyboard.c -o keyboard.o

vga.o: src/drivers/vga/vga.c
	$(CC) $(CFLAGS) -c src/drivers/vga/vga.c -o vga.o

# --- НОВОЕ: Менеджер команд (Shell) ---
shell.o: src/shell/shell.c
	$(CC) $(CFLAGS) -c src/shell/shell.c -o shell.o

gdt.o: src/system/gdt.c
	$(CC) $(CFLAGS) -c src/system/gdt.c -o gdt.o

gdt_flush.o: src/system/gdt_flush.asm
	$(ASM) -f elf32 src/system/gdt_flush.asm -o gdt_flush.o

idt.o: src/system/idt.c
	$(CC) $(CFLAGS) -c src/system/idt.c -o idt.o

pic.o: src/system/pic.c
	$(CC) $(CFLAGS) -c src/system/pic.c -o pic.o

interrupt.o: src/system/interrupt.asm
	$(ASM) -f elf32 src/system/interrupt.asm -o interrupt.o

font.o: src/drivers/vga/font.psf
	objcopy -I binary -O pe-i386 -B i386 src/drivers/vga/font.psf font.o

timer.o: src/system/timer.c
	$(CC) $(CFLAGS) -c src/system/timer.c -o timer.o

ata.o: src/drivers/disk/ata.c
	$(CC) $(CFLAGS) -c src/drivers/disk/ata.c -o ata.o

memory.o: src/system/memory.c
	$(CC) $(CFLAGS) -c src/system/memory.c -o memory.o

fs.o: src/fs/fs.c
	$(CC) $(CFLAGS) -c src/fs/fs.c -o fs.o

# 5. Линковка (kernel_entry.o ВСЕГДА ПЕРВЫЙ!)
kernel.bin: $(OBJ)
	$(LD) -m i386pe --image-base 0 -T src/linker.ld $(OBJ) -o kernel.tmp
	$(OBJCOPY) -O binary kernel.tmp kernel.bin

# 6. Склейка образа
os-image.bin: boot.bin kernel.bin
	copy /b boot.bin + kernel.bin os-image.bin

run:
	$(EMU) -drive format=raw,file=os-image.bin,index=0,media=disk -drive format=raw,file=fs.img,index=1,media=disk

clean:
	del *.bin *.o *.tmp