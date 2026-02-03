# EquinoxOS
### A hobby operating system inspired by Windows 7 theme (Frutiger Aero).

## Current Features:
- 16-bit Bootloader
- 32-bit Protected Mode Kernel
- GDT & IDT Initialization
- VGA Screen Driver with Hardware Cursor
- Keyboard Driver (ASCII, Shift support)

## Tech Stack:
- **Language:** C, x86 Assembly
- **Compiler:** GCC (MinGW-w64), i686, x86_64-elf
- **Assembler:** NASM
- **Emulator:** QEMU

to build:

xorriso -as mkisofs -b limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table -o equos.iso iso_root
.\limine.exe bios-install equos.iso
qemu-system-x86_64 -cdrom equos.iso

if it doesnt work:
xorriso -as mkisofs -b limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table -o equos.iso iso_root

Made with help of AI.