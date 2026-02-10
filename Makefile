CC = x86_64-elf-gcc   # Сменил
LD = x86_64-elf-ld    # Сменил
ASM = nasm
OBJCOPY = x86_64-elf-objcopy # Сменил

CFLAGS = -ffreestanding -O2 -Wall -Wextra -fno-exceptions -std=c11 -Isrc -Isrc/drivers -Isrc/shell -Isrc/boot/limine -mcmodel=large
LDFLAGS = -nostdlib -T src/linker.ld -z max-page-size=0x1000
ASMFLAGS = -f elf64

OBJ_DIR = obj
OBJ = $(OBJ_DIR)/kernel.o $(OBJ_DIR)/io.o $(OBJ_DIR)/keyboard.o \
      $(OBJ_DIR)/gdt_flush.o $(OBJ_DIR)/idt.o \
      $(OBJ_DIR)/pic.o $(OBJ_DIR)/interrupt.o $(OBJ_DIR)/timer.o $(OBJ_DIR)/ata.o \
      $(OBJ_DIR)/memory.o $(OBJ_DIR)/fs.o $(OBJ_DIR)/vesa.o $(OBJ_DIR)/mouse.o $(OBJ_DIR)/string.o

all: setup kernel.elf

setup:
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)

kernel.elf: $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) -o kernel.elf

$(OBJ_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/system/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/drivers/screen/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/drivers/keyboard/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/shell/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/drivers/disk/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/fs/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/drivers/vga/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/io/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/drivers/mouse/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/gdt_flush.o: src/system/gdt_flush.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(OBJ_DIR)/interrupt.o: src/system/interrupt.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	@if exist $(OBJ_DIR) rmdir /s /q $(OBJ_DIR)
	@if exist kernel.elf del kernel.elf